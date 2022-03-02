// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_TerrainMaterialInstance.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_MaterialBase* UAGX_TerrainMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_TerrainMaterialInstance* InstancePtr = TerrainMaterialInstance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TerrainMaterialInstance::CreateFromAsset(PlayingWorld, this);
		TerrainMaterialInstance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_TerrainMaterialAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// The name of the property that was changed.
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// The root property that contains the property that was changed.
	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL)
								   ? PropertyChangedEvent.MemberProperty->GetFName()
								   : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface))
	{
		WriteSurfacePropertyToInstance(PropertyName);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk))
	{
		WriteBulkPropertyToInstance(PropertyName);
	}
	else if (
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction))
	{
		WriteCompactionPropertyToInstance(PropertyName);
	}
}

void UAGX_TerrainMaterialAsset::WriteSurfacePropertyToInstance(const FName& PropertyName)
{
	UAGX_TerrainMaterialInstance* Instance = TerrainMaterialInstance.Get();
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_ShapeMaterialSurfaceProperties SurfaceProperties;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, bFrictionEnabled))
	{
		Instance->SetFrictionEnabled(Surface.bFrictionEnabled);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, Roughness))
	{
		Instance->SetRoughness(static_cast<float>(Surface.Roughness));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, Viscosity))
	{
		Instance->SetSurfaceViscosity(static_cast<float>(Surface.Viscosity));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, AdhesiveForce))
	{
		Instance->SetAdhesion(
			static_cast<float>(Surface.AdhesiveForce), static_cast<float>(Surface.AdhesiveOverlap));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, AdhesiveOverlap))
	{
		Instance->SetAdhesion(
			static_cast<float>(Surface.AdhesiveForce), static_cast<float>(Surface.AdhesiveOverlap));
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the TerrainMaterialInstance."),
			*PropertyName.ToString());
	}
}

void UAGX_TerrainMaterialAsset::WriteBulkPropertyToInstance(const FName& PropertyName)
{
	UAGX_TerrainMaterialInstance* Instance = TerrainMaterialInstance.Get();
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_TerrainBulkProperties BulkProperties;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, AdhesionOverlapFactor))
	{
		Instance->SetAdhesionOverlapFactor(static_cast<float>(TerrainBulk.AdhesionOverlapFactor));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, Cohesion))
	{
		Instance->SetCohesion(static_cast<float>(TerrainBulk.Cohesion));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, Density))
	{
		Instance->SetDensity(static_cast<float>(TerrainBulk.Density));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, DilatancyAngle))
	{
		Instance->SetDilatancyAngle(static_cast<float>(TerrainBulk.DilatancyAngle));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, FrictionAngle))
	{
		Instance->SetFrictionAngle(static_cast<float>(TerrainBulk.FrictionAngle));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, MaxDensity))
	{
		Instance->SetMaxDensity(static_cast<float>(TerrainBulk.MaxDensity));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, PoissonsRatio))
	{
		Instance->SetPoissonsRatio(static_cast<float>(TerrainBulk.PoissonsRatio));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, SwellFactor))
	{
		Instance->SetSwellFactor(static_cast<float>(TerrainBulk.SwellFactor));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, YoungsModulus))
	{
		Instance->SetYoungsModulus(static_cast<float>(TerrainBulk.YoungsModulus));
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the TerrainMaterialInstance."),
			*PropertyName.ToString());
	}
}

void UAGX_TerrainMaterialAsset::WriteCompactionPropertyToInstance(const FName& PropertyName)
{
	UAGX_TerrainMaterialInstance* Instance = TerrainMaterialInstance.Get();
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_TerrainCompactionProperties CompactionProps;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, AngleOfReposeCompactionRate))
	{
		Instance->SetAngleOfReposeCompactionRate(
			static_cast<float>(TerrainCompaction.AngleOfReposeCompactionRate));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, Phi0))
	{
		Instance->SetBankStatePhi(static_cast<float>(TerrainCompaction.Phi0));
	}
	else if (
		PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, CompactionTimeRelaxationConstant))
	{
		Instance->SetCompactionTimeRelaxationConstant(
			static_cast<float>(TerrainCompaction.CompactionTimeRelaxationConstant));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, CompressionIndex))
	{
		Instance->SetCompressionIndex(static_cast<float>(TerrainCompaction.CompressionIndex));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, K_e))
	{
		Instance->SetHardeningConstantKe(static_cast<float>(TerrainCompaction.K_e));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, N_e))
	{
		Instance->SetHardeningConstantNe(static_cast<float>(TerrainCompaction.N_e));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, PreconsolidationStress))
	{
		Instance->SetPreconsolidationStress(
			static_cast<float>(TerrainCompaction.PreconsolidationStress));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(CompactionProps, StressCutOffFraction))
	{
		Instance->SetStressCutOffFraction(
			static_cast<float>(TerrainCompaction.StressCutOffFraction));
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the TerrainMaterialInstance."),
			*PropertyName.ToString());
	}
}
#endif
