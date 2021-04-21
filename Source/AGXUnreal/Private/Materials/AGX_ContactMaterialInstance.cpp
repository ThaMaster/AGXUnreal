#include "Materials/AGX_ContactMaterialInstance.h"

#include "Engine/World.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/ShapeMaterialBarrier.h"

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ContactMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_ContactMaterialInstance* NewInstance = NewObject<UAGX_ContactMaterialInstance>(
		Outer, UAGX_ContactMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyFrom(Source);
	NewInstance->SourceAsset = Source;

	NewInstance->CreateNative(PlayingWorld);

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
}

void UAGX_ContactMaterialInstance::SetSurfaceFrictionEnabled(bool bInSurfaceFrictionEnabled)
{
	Super::SetSurfaceFrictionEnabled(bInSurfaceFrictionEnabled);
	if  (!HasNative())
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

void UAGX_ContactMaterialInstance::SetSecondaryFrictionCoefficient(float InSecondaryFrictionCoefficient)
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

void UAGX_ContactMaterialInstance::SetUseSecondaryFrictionCoefficient(bool bInUseSecondaryFrictionCoefficient)
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
	NativeBarrier->SetSurfaceViscosity(
		SurfaceViscosity, true, !bUseSecondarySurfaceViscosity);
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

void UAGX_ContactMaterialInstance::SetUseSecondarySurfaceViscosity(bool bInUseSecondarySurfaceViscosity)
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

void UAGX_ContactMaterialInstance::SetRestitution(float Restitution)
{
	Super::SetRestitution(Restitution);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetRestitution(Restitution);
}

void UAGX_ContactMaterialInstance::SetDamping(float Damping)
{
	Super::SetDamping(Damping);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetDamping(Damping);
}

void UAGX_ContactMaterialInstance::SetYoungsModulus(float YoungsModulus)
{
	Super::SetYoungsModulus(YoungsModulus);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetYoungsModulus(YoungsModulus);
}

void UAGX_ContactMaterialInstance::SetAdhesiveForce(float AdhesiveForce)
{
	Super::SetAdhesiveForce(AdhesiveForce);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
}

void UAGX_ContactMaterialInstance::SetAdhesiveOverlap(float AdhesiveOverlap)
{
	Super::SetAdhesiveOverlap(AdhesiveOverlap);
	if (!HasNative())
	{
		return;
	}
	NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
}


FContactMaterialBarrier* UAGX_ContactMaterialInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
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

void UAGX_ContactMaterialInstance::UpdateNativeProperties()
{
	if (HasNative())
	{
		// Friction related properties
		{
			/// \note Setting Friction Model before Solve Type, because Solve Type is part of the
			/// Friction Model object.
			NativeBarrier->SetFrictionModel(static_cast<uint32>(FrictionModel));
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
			NativeBarrier->SetDamping(Damping);
			NativeBarrier->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
		}
	}
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::GetInstance()
{
	return this;
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	return this;
};

void UAGX_ContactMaterialInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FContactMaterialBarrier());

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

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Simulation);

	Simulation->GetNative()->AddContactMaterial(NativeBarrier.Get());
}
