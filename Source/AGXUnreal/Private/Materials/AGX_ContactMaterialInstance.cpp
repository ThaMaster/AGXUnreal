// Fill out your copyright notice in the Description page of Project Settings.

#include "Materials/AGX_ContactMaterialInstance.h"

#include "Engine/World.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/MaterialBarrier.h"

UAGX_ContactMaterialInstance* UAGX_ContactMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ContactMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_ContactMaterialBase::CreateFromAsset is creating an instance named \"%s\" (from "
			 "asset \"%s\")."),
		*InstanceName, *Source->GetName());

	UAGX_ContactMaterialInstance* NewInstance = NewObject<UAGX_ContactMaterialInstance>(
		Outer, UAGX_ContactMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyProperties(Source);
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
				NativeBarrier->SetSurfaceViscosity(
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

	UAGX_ShapeMaterialInstance* MaterialInstance1 =
		UAGX_MaterialBase::GetOrCreateShapeMaterialInstance(GetWorld(), Material1);
	UAGX_ShapeMaterialInstance* MaterialInstance2 =
		UAGX_MaterialBase::GetOrCreateShapeMaterialInstance(GetWorld(), Material2);

	FMaterialBarrier* MaterialBarrier1 =
		MaterialInstance1 ? MaterialInstance1->GetOrCreateNative(GetWorld()) : nullptr;
	FMaterialBarrier* MaterialBarrier2 =
		MaterialInstance2 ? MaterialInstance2->GetOrCreateNative(GetWorld()) : nullptr;

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
