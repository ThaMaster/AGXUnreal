#include "Materials/AGX_TerrainMaterialInstance.h"

#include "Engine/World.h"

#include "Materials/AGX_TerrainMaterialAsset.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

UAGX_TerrainMaterialInstance::~UAGX_TerrainMaterialInstance()
{
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetOrCreateTerrainMaterialNative(
	UWorld* PlayingWorld)
{
	if (!HasTerrainMaterialNative())
	{
		CreateTerrainMaterialNative(PlayingWorld);
	}
	return GetTerrainMaterialNative();
}

FMaterialBarrier* UAGX_TerrainMaterialInstance::GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (!HasShapeMaterialNative())
	{
		CreateShapeMaterialNative(PlayingWorld);
	}
	return GetShapeMaterialNative();
}

UAGX_MaterialBase* UAGX_TerrainMaterialInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_TerrainMaterialInstance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_TerrainMaterialInstance::CreateFromAsset is creating an instance "
			 "named \"%s\" (from asset  %s)."),
		*InstanceName, *Source->GetName());

	UAGX_TerrainMaterialInstance* NewInstance = NewObject<UAGX_TerrainMaterialInstance>(
		Outer, UAGX_TerrainMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	// Copy the terrain material properties
	NewInstance->CopyTerrainMaterialProperties(Source);

	NewInstance->CreateTerrainMaterialNative(PlayingWorld);

	return NewInstance;
}

void UAGX_TerrainMaterialInstance::CreateTerrainMaterialNative(UWorld* PlayingWorld)
{
	TerrainMaterialNativeBarrier.Reset(new FTerrainMaterialBarrier());

	TerrainMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasTerrainMaterialNative());

	UpdateTerrainMaterialNativeProperties();
}

void UAGX_TerrainMaterialInstance::CreateShapeMaterialNative(UWorld* PlayingWorld)
{
	ShapeMaterialNativeBarrier.Reset(new FMaterialBarrier());

	ShapeMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasShapeMaterialNative());

	UpdateShapeMaterialNativeProperties();
}

bool UAGX_TerrainMaterialInstance::HasTerrainMaterialNative() const
{
	return TerrainMaterialNativeBarrier && TerrainMaterialNativeBarrier->HasNative();
}

bool UAGX_TerrainMaterialInstance::HasShapeMaterialNative() const
{
	return ShapeMaterialNativeBarrier && ShapeMaterialNativeBarrier->HasNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetTerrainMaterialNative()
{
	if (TerrainMaterialNativeBarrier)
	{
		return TerrainMaterialNativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

FMaterialBarrier* UAGX_TerrainMaterialInstance::GetShapeMaterialNative()
{
	if (ShapeMaterialNativeBarrier)
	{
		return ShapeMaterialNativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

void UAGX_TerrainMaterialInstance::UpdateTerrainMaterialNativeProperties()
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		// Set Bulk properties.
		TerrainMaterialNativeBarrier->SetAdhesionOverlapFactor(TerrainBulk.AdhesionOverlapFactor);
		TerrainMaterialNativeBarrier->SetCohesion(TerrainBulk.Cohesion);
		TerrainMaterialNativeBarrier->SetDensity(TerrainBulk.Density);
		TerrainMaterialNativeBarrier->SetDilatancyAngle(TerrainBulk.DilatancyAngle);
		TerrainMaterialNativeBarrier->SetFrictionAngle(TerrainBulk.FrictionAngle);
		TerrainMaterialNativeBarrier->SetMaximumDensity(TerrainBulk.MaxDensity);
		TerrainMaterialNativeBarrier->SetPoissonsRatio(TerrainBulk.PoissonsRatio);
		TerrainMaterialNativeBarrier->SetSwellFactor(TerrainBulk.SwellFactor);
		TerrainMaterialNativeBarrier->SetYoungsModulus(TerrainBulk.YoungsModulus);

		// Set Compaction properties.
		TerrainMaterialNativeBarrier->SetAngleOfReposeCompactionRate(
			TerrainCompaction.AngleOfReposeCompactionRate);
		TerrainMaterialNativeBarrier->SetBankStatePhi(TerrainCompaction.Phi0);
		TerrainMaterialNativeBarrier->SetCompactionTimeRelaxationConstant(
			TerrainCompaction.CompactionTimeRelaxationConstant);
		TerrainMaterialNativeBarrier->SetCompressionIndex(TerrainCompaction.CompressionIndex);
		TerrainMaterialNativeBarrier->SetHardeningConstantKE(TerrainCompaction.K_e);
		TerrainMaterialNativeBarrier->SetHardeningConstantNE(TerrainCompaction.N_e);
		TerrainMaterialNativeBarrier->SetPreconsolidationStress(
			TerrainCompaction.PreconsolidationStress);
		TerrainMaterialNativeBarrier->SetStressCutOffFraction(
			TerrainCompaction.StressCutOffFraction);
	}
}

void UAGX_TerrainMaterialInstance::UpdateShapeMaterialNativeProperties()
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		ShapeMaterialNativeBarrier->SetDensity(Bulk.Density);
		ShapeMaterialNativeBarrier->SetYoungsModulus(Bulk.YoungsModulus);
		ShapeMaterialNativeBarrier->SetBulkViscosity(Bulk.Viscosity);
		ShapeMaterialNativeBarrier->SetDamping(Bulk.Damping);
		ShapeMaterialNativeBarrier->SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		ShapeMaterialNativeBarrier->SetFrictionEnabled(Surface.bFrictionEnabled);
		ShapeMaterialNativeBarrier->SetRoughness(Surface.Roughness);
		ShapeMaterialNativeBarrier->SetSurfaceViscosity(Surface.Viscosity);
		ShapeMaterialNativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
	}
}
