// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "EngineUtils.h"

namespace AGX_ContactMaterialInstance_helpers
{
	AActor* FindActorByName(UWorld* World, const FName& ActorName)
	{
		check(World);
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetFName().IsEqual(ActorName, ENameCase::CaseSensitive))
			{
				return *It;
			}
		}
		return nullptr;
	}

	/**
	 * Finds and the component that should be used as oriented friction model Reference Frame,
	 * given a component name, owning actor name (optional), and the
	 * ContactMaterialRegistrarComponent that registers the contact material.
	 *
	 * If the actor name is is empty the component will be searched for in actor owning the
	 * ContactMaterialRegistrarComponent.
	 *
	 * Currently the component must be a UAGX_RigidBodyComponent.
	 */
	UAGX_RigidBodyComponent* FindReferenceFrameComponent(
		const FName& ComponentName, const FName& ActorName,
		UAGX_ContactMaterialRegistrarComponent* Registrar)
	{
		check(Registrar);
		check(Registrar->GetTypedOuter<AActor>());

		if (ComponentName.IsNone())
			return nullptr;

		AActor* OwningActor = ActorName.IsNone()
								  ? Registrar->GetTypedOuter<AActor>()
								  : FindActorByName(Registrar->GetWorld(), ActorName);

		if (OwningActor == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FindReferenceFrame() failed to find reference frame component matching "
					 "ComponentName: '%s', ActorName: '%s', Registrar: '%s' in '%s' because the "
					 "owning actor could not be found."),
				*ComponentName.ToString(), *ActorName.ToString(), *Registrar->GetName(),
				*GetFNameSafe(Registrar->GetOwner()).ToString());
			return nullptr;
		}

		TArray<UAGX_RigidBodyComponent*> Bodies;
		OwningActor->GetComponents(Bodies);
		UAGX_RigidBodyComponent** It =
			Bodies.FindByPredicate([ComponentName](UAGX_RigidBodyComponent* Body) {
				return Body->GetFName().IsEqual(ComponentName, ENameCase::CaseSensitive);
			});
		if (It == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FindReferenceFrame() failed to find reference frame component matching "
					 "ComponentName: '%s', ActorName: '%s', Registrar: '%s' in '%s' because the "
					 "component could not be found inside the actor."),
				*ComponentName.ToString(), *ActorName.ToString(), *Registrar->GetName(),
				*GetFNameSafe(Registrar->GetOwner()).ToString());
			return nullptr;
		}
		return *It;
	}
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::CreateFromAsset(
	UAGX_ContactMaterialRegistrarComponent* Registrar, UAGX_ContactMaterialAsset* Source)
{
	check(Source);
	check(Registrar);

	UWorld* PlayingWorld = Registrar->GetWorld();
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_ContactMaterialInstance* NewInstance = NewObject<UAGX_ContactMaterialInstance>(
		Outer, UAGX_ContactMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyFrom(Source);
	NewInstance->SourceAsset = Source;

	NewInstance->CreateNative(Registrar);

	return NewInstance;
}

UAGX_ContactMaterialInstance::~UAGX_ContactMaterialInstance()
{
}

UAGX_ContactMaterialAsset* UAGX_ContactMaterialInstance::GetAsset()
{
	return SourceAsset.Get();
}

void UAGX_ContactMaterialInstance::SetContactSolver(EAGX_ContactSolver InContactSolver)
{
	Super::SetContactSolver(InContactSolver);
	if (!HasNative())
	{
		return;
	}

	/// @todo This static_cast is suspicious. Consider passing the enum a bit further.
	NativeBarrier->SetFrictionSolveType(static_cast<int32>(ContactSolver));
}

void UAGX_ContactMaterialInstance::SetContactReductionMode(
	EAGX_ContactReductionMode InReductionMode)
{
	Super::SetContactReductionMode(InReductionMode);
	if (!HasNative())
	{
		return;
	}

	/// @todo This static_cast is suspicious. Consider passing the enum a bit further.
	NativeBarrier->SetContactReductionMode(static_cast<int32>(ContactReduction.Mode));
}

void UAGX_ContactMaterialInstance::SetContactReductionBinResolution(uint8 InBinResolution)
{
	Super::SetContactReductionBinResolution(InBinResolution);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetContactReductionBinResolution(ContactReduction.BinResolution);
}

void UAGX_ContactMaterialInstance::SetUseContactAreaApproach(bool bInUseContactAreaApproach)
{
	Super::SetUseContactAreaApproach(bInUseContactAreaApproach);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetUseContactAreaApproach(MechanicsApproach.bUseContactAreaApproach);
}

void UAGX_ContactMaterialInstance::SetMinElasticRestLength(float InMinLength)
{
	Super::SetMinElasticRestLength(InMinLength);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetMinMaxElasticRestLength(
		MechanicsApproach.MinElasticRestLength, MechanicsApproach.MaxElasticRestLength);
}

void UAGX_ContactMaterialInstance::SetMaxElasticRestLength(float InMaxLength)
{
	Super::SetMaxElasticRestLength(InMaxLength);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetMinMaxElasticRestLength(
		MechanicsApproach.MinElasticRestLength, MechanicsApproach.MaxElasticRestLength);
}

void UAGX_ContactMaterialInstance::SetFrictionModel(EAGX_FrictionModel InFrictionModel)
{
	Super::SetFrictionModel(InFrictionModel);
	if (!HasNative())
	{
		return;
	}

	/// @todo This static_cast is suspicious. Consider passing the enum a bit further.
	NativeBarrier->SetFrictionModel(static_cast<int32>(FrictionModel));

	// If friction model changes, re-set friction model dependent parameters.
	if (FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		NativeBarrier->SetNormalForceMagnitude(NormalForceMagnitude);
		NativeBarrier->SetEnableScaleNormalForceWithDepth(bScaleNormalForceWithDepth);
	}
	if (IsOrientedFrictionModel())
	{
		NativeBarrier->SetPrimaryDirection(PrimaryDirection);
	}
}

void UAGX_ContactMaterialInstance::SetNormalForceMagnitude(float InNormalForceMagnitude)
{
	Super::SetNormalForceMagnitude(InNormalForceMagnitude);
	if (!HasNative())
	{
		return;
	}
	if (FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		NativeBarrier->SetNormalForceMagnitude(InNormalForceMagnitude);
	}
}

void UAGX_ContactMaterialInstance::SetScaleNormalForceWithDepth(bool bEnabled)
{
	Super::SetScaleNormalForceWithDepth(bEnabled);
	if (!HasNative())
	{
		return;
	}
	if (FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		NativeBarrier->SetEnableScaleNormalForceWithDepth(bEnabled);
	}
}

void UAGX_ContactMaterialInstance::SetSurfaceFrictionEnabled(bool bInSurfaceFrictionEnabled)
{
	Super::SetSurfaceFrictionEnabled(bInSurfaceFrictionEnabled);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetSurfaceFrictionEnabled(bSurfaceFrictionEnabled);
}

void UAGX_ContactMaterialInstance::SetFrictionCoefficient(float InFrictionCoefficient)
{
	Super::SetFrictionCoefficient(InFrictionCoefficient);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetFrictionCoefficient(
		FrictionCoefficient, true, !bUseSecondaryFrictionCoefficient);
}

void UAGX_ContactMaterialInstance::SetSecondaryFrictionCoefficient(
	float InSecondaryFrictionCoefficient)
{
	Super::SetSecondaryFrictionCoefficient(InSecondaryFrictionCoefficient);
	if (!HasNative())
	{
		return;
	}
	if (bUseSecondaryFrictionCoefficient)
	{
		NativeBarrier->SetFrictionCoefficient(SecondaryFrictionCoefficient, false, true);
	}
}

void UAGX_ContactMaterialInstance::SetUseSecondaryFrictionCoefficient(
	bool bInUseSecondaryFrictionCoefficient)
{
	Super::SetUseSecondaryFrictionCoefficient(bInUseSecondaryFrictionCoefficient);
	if (!HasNative())
	{
		return;
	}
	if (bUseSecondaryFrictionCoefficient)
	{
		NativeBarrier->SetFrictionCoefficient(FrictionCoefficient, true, false);
		NativeBarrier->SetFrictionCoefficient(SecondaryFrictionCoefficient, false, true);
	}
	else
	{
		NativeBarrier->SetFrictionCoefficient(FrictionCoefficient, true, true);
	}
}

void UAGX_ContactMaterialInstance::SetSurfaceViscosity(float InSurfaceViscosity)
{
	Super::SetSurfaceViscosity(InSurfaceViscosity);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetSurfaceViscosity(SurfaceViscosity, true, !bUseSecondarySurfaceViscosity);
}

void UAGX_ContactMaterialInstance::SetSecondarySurfaceViscosity(float InSecondarySurfaceViscosity)
{
	Super::SetSecondarySurfaceViscosity(InSecondarySurfaceViscosity);
	if (!HasNative())
	{
		return;
	}
	if (bUseSecondarySurfaceViscosity)
	{
		NativeBarrier->SetSurfaceViscosity(SecondarySurfaceViscosity, false, true);
	}
}

void UAGX_ContactMaterialInstance::SetUseSecondarySurfaceViscosity(
	bool bInUseSecondarySurfaceViscosity)
{
	Super::SetUseSecondarySurfaceViscosity(bInUseSecondarySurfaceViscosity);
	if (!HasNative())
	{
		return;
	}
	if (bUseSecondarySurfaceViscosity)
	{
		NativeBarrier->SetSurfaceViscosity(SurfaceViscosity, true, false);
		NativeBarrier->SetSurfaceViscosity(SecondarySurfaceViscosity, false, true);
	}
	else
	{
		NativeBarrier->SetSurfaceViscosity(SurfaceViscosity, true, true);
	}
}

void UAGX_ContactMaterialInstance::SetPrimaryDirection(const FVector& InPrimaryDirection)
{
	Super::SetPrimaryDirection(InPrimaryDirection);
	if (!HasNative())
	{
		return;
	}
	if (IsOrientedFrictionModel())
	{
		NativeBarrier->SetPrimaryDirection(InPrimaryDirection);
	}
}

void UAGX_ContactMaterialInstance::SetRestitution(float InRestitution)
{
	Super::SetRestitution(InRestitution);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetRestitution(Restitution);
}

void UAGX_ContactMaterialInstance::SetSpookDamping(float InSpookDamping)
{
	Super::SetSpookDamping(InSpookDamping);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetSpookDamping(SpookDamping);
}

void UAGX_ContactMaterialInstance::SetYoungsModulus(float InYoungsModulus)
{
	Super::SetYoungsModulus(InYoungsModulus);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetYoungsModulus(YoungsModulus);
}

void UAGX_ContactMaterialInstance::SetAdhesiveForce(float InAdhesiveForce)
{
	Super::SetAdhesiveForce(InAdhesiveForce);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
}

void UAGX_ContactMaterialInstance::SetAdhesiveOverlap(float InAdhesiveOverlap)
{
	Super::SetAdhesiveOverlap(InAdhesiveOverlap);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
}

FContactMaterialBarrier* UAGX_ContactMaterialInstance::GetOrCreateNative(
	UAGX_ContactMaterialRegistrarComponent* Registrar)
{
	if (!HasNative())
	{
		CreateNative(Registrar);
	}
	return GetNative();
}

FContactMaterialBarrier* UAGX_ContactMaterialInstance::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

bool UAGX_ContactMaterialInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_ContactMaterialInstance::UpdateNativeProperties(
	UAGX_ContactMaterialRegistrarComponent* Registrar)
{
	if (HasNative())
	{
		// Friction related properties
		{
			/// \note Setting Friction Model before Solve Type, NormalForceMagnitude,
			/// \note Setting Friction Model before Solve Type, because Solve Type is part of the
			/// Friction Model object.
			NativeBarrier->SetFrictionModel(static_cast<uint32>(FrictionModel));
			if (FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
			{
				NativeBarrier->SetNormalForceMagnitude(NormalForceMagnitude);
				NativeBarrier->SetEnableScaleNormalForceWithDepth(bScaleNormalForceWithDepth);
			}
			NativeBarrier->SetSurfaceFrictionEnabled(bSurfaceFrictionEnabled);
			NativeBarrier->SetFrictionCoefficient(
				FrictionCoefficient, /*bPrimaryDir*/ true,
				/*bSecondaryDir*/ !bUseSecondaryFrictionCoefficient);
			if (bUseSecondaryFrictionCoefficient)
			{
				NativeBarrier->SetFrictionCoefficient(
					SecondaryFrictionCoefficient, /*bPrimaryDir*/ false, /*bSecondaryDir*/ true);
			}

			NativeBarrier->SetSurfaceViscosity(
				SurfaceViscosity, /*bPrimaryDir*/ true,
				/*bSecondaryDir*/ !bUseSecondarySurfaceViscosity);
			if (bUseSecondarySurfaceViscosity)
			{
				NativeBarrier->SetSurfaceViscosity(
					SecondarySurfaceViscosity, /*bPrimaryDir*/ false, /*bSecondaryDir*/ true);
			}
			// Update properties exclusive to oriented friction models.
			if (IsOrientedFrictionModel())
			{
				NativeBarrier->SetPrimaryDirection(PrimaryDirection);

				// Set reference frame on native
				UAGX_RigidBodyComponent* ReferenceFrameBody =
					AGX_ContactMaterialInstance_helpers::FindReferenceFrameComponent(
						OrientedFrictionReferenceFrameComponent,
						OrientedFrictionReferenceFrameActor, Registrar);
				if (!ReferenceFrameBody)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("ContactMaterial '%s' has a oriented friction model but the component "
							 "to use as Reference Frame could not be found. Oriented friction "
							 "might not work as expected."),
						*GetName());
					NativeBarrier->SetOrientedFrictionModelReferenceFrame(nullptr);
				}
				else
				{
					UE_LOG(
						LogAGX, Log,
						TEXT("Reference Frame of ContactMaterial '%s' is being set to component "
							 "'%s' in '%s'."),
						*GetName(), *ReferenceFrameBody->GetName(),
						*GetFNameSafe(ReferenceFrameBody->GetOwner()).ToString());
					FRigidBodyBarrier* ReferenceFrameBodyBarrier =
						ReferenceFrameBody->GetOrCreateNative();
					check(ReferenceFrameBodyBarrier);
					NativeBarrier->SetOrientedFrictionModelReferenceFrame(
						ReferenceFrameBodyBarrier);
				}
			}
		}

		// Contact Processing related properties
		{
			NativeBarrier->SetFrictionSolveType(static_cast<uint32>(ContactSolver));
			NativeBarrier->SetContactReductionMode(static_cast<int32>(ContactReduction.Mode));
			NativeBarrier->SetContactReductionBinResolution(ContactReduction.BinResolution);
			NativeBarrier->SetUseContactAreaApproach(MechanicsApproach.bUseContactAreaApproach);
			NativeBarrier->SetMinMaxElasticRestLength(
				MechanicsApproach.MinElasticRestLength, MechanicsApproach.MaxElasticRestLength);
		}

		// General properties
		{
			NativeBarrier->SetRestitution(Restitution);
			NativeBarrier->SetYoungsModulus(YoungsModulus);
			NativeBarrier->SetSpookDamping(SpookDamping);
			NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
		}
	}
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::GetInstance()
{
	return this;
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::GetOrCreateInstance(
	UAGX_ContactMaterialRegistrarComponent* Registrar)
{
	return this;
};

void UAGX_ContactMaterialInstance::CreateNative(UAGX_ContactMaterialRegistrarComponent* Registrar)
{
	NativeBarrier.Reset(new FContactMaterialBarrier());

	UWorld* PlayingWorld = Registrar->GetWorld();

	/// \note AGX seems OK with referenced materials being null. Falls back on native default
	/// material.

	UAGX_MaterialBase* MaterialInstance1 = Material1->GetOrCreateInstance(GetWorld());
	UAGX_MaterialBase* MaterialInstance2 = Material2->GetOrCreateInstance(GetWorld());

	// Swap properties
	if (PlayingWorld && PlayingWorld->IsGameWorld())
	{
		if (MaterialInstance1 != Material1)
		{
			Material1 = MaterialInstance1;
		}

		if (MaterialInstance2 != Material2)
		{
			Material2 = MaterialInstance2;
		}
	}

	FShapeMaterialBarrier* MaterialBarrier1 =
		MaterialInstance1 ? MaterialInstance1->GetOrCreateShapeMaterialNative(GetWorld()) : nullptr;
	FShapeMaterialBarrier* MaterialBarrier2 =
		MaterialInstance2 ? MaterialInstance2->GetOrCreateShapeMaterialNative(GetWorld()) : nullptr;

	UE_LOG(
		LogAGX, Log,
		TEXT(
			"UAGX_ContactMaterialInstance::CreateNative is creating native contact material \"%s\" "
			"using materials \"%s\" and \"%s\"."),
		*GetName(), *GetNameSafe(MaterialInstance1), *GetNameSafe(MaterialInstance2));

	NativeBarrier->AllocateNative(
		MaterialBarrier1, MaterialBarrier2); // materials can only be set on construction
	check(HasNative());

	// /todo Cache the Registrar as a ConactMaterialInstance member variable instead of
	//       passing it around as argument?
	UpdateNativeProperties(Registrar);
}
