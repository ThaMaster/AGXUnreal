// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

// Surface properties.
void UAGX_TerrainMaterial::SetFrictionEnabled(bool Enabled)
{
	if (IsInstance())
	{
		Surface.bFrictionEnabled = Enabled;
		if (HasShapeMaterialNative())
			ShapeMaterialNativeBarrier.SetFrictionEnabled(Enabled);
	}
	else // IsAsset
	{
		if (Instance != nullptr)
		{
			Instance->SetFrictionEnabled(Enabled);
			return;
		}
		Surface.bFrictionEnabled = Enabled;
	}
}

bool UAGX_TerrainMaterial::GetFrictionEnabled() const
{
	if (Instance != nullptr)
		return Instance->GetFrictionEnabled();
	if (HasShapeMaterialNative())
		return ShapeMaterialNativeBarrier.GetFrictionEnabled();

	return Surface.bFrictionEnabled;
}

void UAGX_TerrainMaterial::SetRoughness_BP(float Roughness)
{
	SetRoughness(static_cast<double>(Roughness));
}

void UAGX_TerrainMaterial::SetRoughness(double Roughness)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.Roughness, Roughness, SetRoughness, HasShapeMaterialNative,
		ShapeMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetRoughness_BP() const
{
	return static_cast<float>(GetRoughness());
}

double UAGX_TerrainMaterial::GetRoughness() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.Roughness, GetRoughness, HasShapeMaterialNative, ShapeMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetSurfaceViscosity_BP(float Viscosity)
{
	SetSurfaceViscosity(static_cast<double>(Viscosity));
}

void UAGX_TerrainMaterial::SetSurfaceViscosity(double Viscosity)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.Viscosity, Viscosity, SetSurfaceViscosity, HasShapeMaterialNative,
		ShapeMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetSurfaceViscosity_BP() const
{
	return static_cast<float>(GetSurfaceViscosity());
}

double UAGX_TerrainMaterial::GetSurfaceViscosity() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.Viscosity, GetSurfaceViscosity, HasShapeMaterialNative, ShapeMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetAdhesion_BP(float AdhesiveForce, float AdhesiveOverlap)
{
	SetAdhesion(static_cast<double>(AdhesiveForce), static_cast<double>(AdhesiveOverlap));
}

void UAGX_TerrainMaterial::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	if (IsInstance())
	{
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
		if (HasShapeMaterialNative())
		{
			ShapeMaterialNativeBarrier.SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
		}
	}
	else // IsAsset
	{
		if (Instance != nullptr)
		{
			Instance->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
			return;
		}
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
	}
}

float UAGX_TerrainMaterial::GetAdhesiveForce_BP() const
{
	return static_cast<float>(GetAdhesiveForce());
}

double UAGX_TerrainMaterial::GetAdhesiveForce() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.AdhesiveForce, GetAdhesiveForce, HasShapeMaterialNative,
		ShapeMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetAdhesiveOverlap_BP() const
{
	return static_cast<float>(GetAdhesiveOverlap());
}

double UAGX_TerrainMaterial::GetAdhesiveOverlap() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		Surface.AdhesiveOverlap, GetAdhesiveOverlap, HasShapeMaterialNative,
		ShapeMaterialNativeBarrier);
}

// Bulk properties.
void UAGX_TerrainMaterial::SetAdhesionOverlapFactor_BP(float AdhesionOverlapFactor)
{
	SetAdhesionOverlapFactor(static_cast<double>(AdhesionOverlapFactor));
}

void UAGX_TerrainMaterial::SetAdhesionOverlapFactor(double AdhesionOverlapFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.AdhesionOverlapFactor, AdhesionOverlapFactor, SetAdhesionOverlapFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetAdhesionOverlapFactor_BP() const
{
	return static_cast<float>(GetAdhesionOverlapFactor());
}

double UAGX_TerrainMaterial::GetAdhesionOverlapFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.AdhesionOverlapFactor, GetAdhesionOverlapFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCohesion_BP(float Cohesion)
{
	SetCohesion(static_cast<double>(Cohesion));
}

void UAGX_TerrainMaterial::SetCohesion(double Cohesion)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Cohesion, Cohesion, SetCohesion, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetCohesion_BP() const
{
	return static_cast<float>(GetCohesion());
}

double UAGX_TerrainMaterial::GetCohesion() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Cohesion, GetCohesion, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDensity_BP(float Density)
{
	SetDensity(static_cast<double>(Density));
}

void UAGX_TerrainMaterial::SetDensity(double Density)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Density, Density, SetDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetDensity_BP() const
{
	return static_cast<float>(GetDensity());
}

double UAGX_TerrainMaterial::GetDensity() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Density, GetDensity, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDilatancyAngle_BP(float DilatancyAngle)
{
	SetDilatancyAngle(static_cast<double>(DilatancyAngle));
}

void UAGX_TerrainMaterial::SetDilatancyAngle(double DilatancyAngle)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.DilatancyAngle, DilatancyAngle, SetDilatancyAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetDilatancyAngle_BP() const
{
	return static_cast<float>(GetDilatancyAngle());
}

double UAGX_TerrainMaterial::GetDilatancyAngle() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.DilatancyAngle, GetDilatancyAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetFrictionAngle_BP(float FrictionAngle)
{
	SetFrictionAngle(static_cast<double>(FrictionAngle));
}

void UAGX_TerrainMaterial::SetFrictionAngle(double FrictionAngle)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.FrictionAngle, FrictionAngle, SetFrictionAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetFrictionAngle_BP() const
{
	return static_cast<float>(GetFrictionAngle());
}

double UAGX_TerrainMaterial::GetFrictionAngle() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.FrictionAngle, GetFrictionAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetMaxDensity_BP(float MaxDensity)
{
	SetMaxDensity(static_cast<double>(MaxDensity));
}

void UAGX_TerrainMaterial::SetMaxDensity(double MaxDensity)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.MaxDensity, MaxDensity, SetMaxDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetMaxDensity_BP() const
{
	return static_cast<float>(GetMaxDensity());
}

double UAGX_TerrainMaterial::GetMaxDensity() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.MaxDensity, GetMaxDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetPoissonsRatio_BP(float PoissonsRatio)
{
	SetPoissonsRatio(static_cast<double>(PoissonsRatio));
}

void UAGX_TerrainMaterial::SetPoissonsRatio(double PoissonsRatio)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.PoissonsRatio, PoissonsRatio, SetPoissonsRatio, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetPoissonsRatio_BP() const
{
	return static_cast<float>(GetPoissonsRatio());
}

double UAGX_TerrainMaterial::GetPoissonsRatio() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.PoissonsRatio, GetPoissonsRatio, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetSwellFactor_BP(float SwellFactor)
{
	SetSwellFactor(static_cast<double>(SwellFactor));
}

void UAGX_TerrainMaterial::SetSwellFactor(double SwellFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.SwellFactor, SwellFactor, SetSwellFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetSwellFactor_BP() const
{
	return static_cast<float>(GetSwellFactor());
}

double UAGX_TerrainMaterial::GetSwellFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.SwellFactor, GetSwellFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetYoungsModulus_BP(float YoungsModulus)
{
	SetYoungsModulus(static_cast<double>(YoungsModulus));
}

void UAGX_TerrainMaterial::SetYoungsModulus(double YoungsModulus)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.YoungsModulus, YoungsModulus, SetYoungsModulus, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetYoungsModulus_BP() const
{
	return static_cast<float>(GetYoungsModulus());
}

double UAGX_TerrainMaterial::GetYoungsModulus() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.YoungsModulus, GetYoungsModulus, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

// Compaction properties.
void UAGX_TerrainMaterial::SetAngleOfReposeCompactionRate_BP(float AngleOfReposeCompactionRate)
{
	SetAngleOfReposeCompactionRate(static_cast<double>(AngleOfReposeCompactionRate));
}

void UAGX_TerrainMaterial::SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.AngleOfReposeCompactionRate, AngleOfReposeCompactionRate,
		SetAngleOfReposeCompactionRate, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetAngleOfReposeCompactionRate_BP() const
{
	return static_cast<float>(GetAngleOfReposeCompactionRate());
}

double UAGX_TerrainMaterial::GetAngleOfReposeCompactionRate() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.AngleOfReposeCompactionRate, GetAngleOfReposeCompactionRate,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetBankStatePhi_BP(float Phi0)
{
	SetBankStatePhi(static_cast<double>(Phi0));
}

void UAGX_TerrainMaterial::SetBankStatePhi(double Phi0)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.BankStatePhi0, Phi0, SetBankStatePhi, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetBankStatePhi_BP() const
{
	return static_cast<float>(GetBankStatePhi());
}

double UAGX_TerrainMaterial::GetBankStatePhi() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.BankStatePhi0, GetBankStatePhi, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCompactionTimeRelaxationConstant_BP(
	float CompactionTimeRelaxationConstant)
{
	SetCompactionTimeRelaxationConstant(static_cast<double>(CompactionTimeRelaxationConstant));
}

void UAGX_TerrainMaterial::SetCompactionTimeRelaxationConstant(
	double CompactionTimeRelaxationConstant)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompactionTimeRelaxationConstant, CompactionTimeRelaxationConstant,
		SetCompactionTimeRelaxationConstant, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetCompactionTimeRelaxationConstant_BP() const
{
	return static_cast<float>(GetCompactionTimeRelaxationConstant());
}

double UAGX_TerrainMaterial::GetCompactionTimeRelaxationConstant() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompactionTimeRelaxationConstant, GetCompactionTimeRelaxationConstant,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCompressionIndex_BP(float CompressionIndex)
{
	SetCompressionIndex(static_cast<double>(CompressionIndex));
}

void UAGX_TerrainMaterial::SetCompressionIndex(double CompressionIndex)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompressionIndex, CompressionIndex, SetCompressionIndex,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetCompressionIndex_BP() const
{
	return static_cast<float>(GetCompressionIndex());
}

double UAGX_TerrainMaterial::GetCompressionIndex() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompressionIndex, GetCompressionIndex, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetHardeningConstantKe_BP(float Ke)
{
	SetHardeningConstantKe(static_cast<double>(Ke));
}

void UAGX_TerrainMaterial::SetHardeningConstantKe(double Ke)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantKe, Ke, SetHardeningConstantKe,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetHardeningConstantKe_BP() const
{
	return static_cast<float>(GetHardeningConstantKe());
}

double UAGX_TerrainMaterial::GetHardeningConstantKe() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantKe, GetHardeningConstantKe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetHardeningConstantNe_BP(float Ne)
{
	SetHardeningConstantNe(static_cast<double>(Ne));
}

void UAGX_TerrainMaterial::SetHardeningConstantNe(double Ne)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantNe, Ne, SetHardeningConstantNe,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetHardeningConstantNe_BP() const
{
	return static_cast<float>(GetHardeningConstantNe());
}

double UAGX_TerrainMaterial::GetHardeningConstantNe() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantNe, GetHardeningConstantNe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetPreconsolidationStress_BP(float PreconsolidationStress)
{
	SetPreconsolidationStress(static_cast<double>(PreconsolidationStress));
}

void UAGX_TerrainMaterial::SetPreconsolidationStress(double PreconsolidationStress)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.PreconsolidationStress, PreconsolidationStress, SetPreconsolidationStress,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetPreconsolidationStress_BP() const
{
	return static_cast<float>(GetPreconsolidationStress());
}

double UAGX_TerrainMaterial::GetPreconsolidationStress() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.PreconsolidationStress, GetPreconsolidationStress,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetStressCutOffFraction_BP(float StressCutOffFraction)
{
	SetStressCutOffFraction(static_cast<double>(StressCutOffFraction));
}

void UAGX_TerrainMaterial::SetStressCutOffFraction(double StressCutOffFraction)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.StressCutOffFraction, StressCutOffFraction, SetStressCutOffFraction,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetStressCutOffFraction_BP() const
{
	return static_cast<float>(GetStressCutOffFraction());
}

double UAGX_TerrainMaterial::GetStressCutOffFraction() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.StressCutOffFraction, GetStressCutOffFraction, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDilatancyAngleScalingFactor_BP(float DilatancyAngleScalingFactor)
{
	SetDilatancyAngleScalingFactor(static_cast<double>(DilatancyAngleScalingFactor));
}

void UAGX_TerrainMaterial::SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.DilatancyAngleScalingFactor, DilatancyAngleScalingFactor,
		SetDilatancyAngleScalingFactor, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

float UAGX_TerrainMaterial::GetDilatancyAngleScalingFactor_BP() const
{
	return static_cast<float>(GetDilatancyAngleScalingFactor());
}

double UAGX_TerrainMaterial::GetDilatancyAngleScalingFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.DilatancyAngleScalingFactor, GetDilatancyAngleScalingFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	TerrainCompaction.Serialize(Archive);
}

void UAGX_TerrainMaterial::CopyFrom(const FTerrainMaterialBarrier& Source)
{
	TerrainBulk = FAGX_TerrainBulkProperties();
	TerrainBulk.AdhesionOverlapFactor = Source.GetAdhesionOverlapFactor();
	TerrainBulk.Cohesion = Source.GetCohesion();
	TerrainBulk.Density = Source.GetDensity();
	TerrainBulk.DilatancyAngle = Source.GetDilatancyAngle();
	TerrainBulk.FrictionAngle = Source.GetFrictionAngle();
	TerrainBulk.MaxDensity = Source.GetMaxDensity();
	TerrainBulk.PoissonsRatio = Source.GetPoissonsRatio();
	TerrainBulk.SwellFactor = Source.GetSwellFactor();
	TerrainBulk.YoungsModulus = Source.GetYoungsModulus();

	TerrainCompaction = FAGX_TerrainCompactionProperties();
	TerrainCompaction.AngleOfReposeCompactionRate = Source.GetAngleOfReposeCompactionRate();
	TerrainCompaction.BankStatePhi0 = Source.GetBankStatePhi();
	TerrainCompaction.CompactionTimeRelaxationConstant =
		Source.GetCompactionTimeRelaxationConstant();
	TerrainCompaction.CompressionIndex = Source.GetCompressionIndex();
	TerrainCompaction.HardeningConstantKe = Source.GetHardeningConstantKe();
	TerrainCompaction.HardeningConstantNe = Source.GetHardeningConstantNe();
	TerrainCompaction.PreconsolidationStress = Source.GetPreconsolidationStress();
	TerrainCompaction.StressCutOffFraction = Source.GetStressCutOffFraction();
	TerrainCompaction.DilatancyAngleScalingFactor = Source.GetDilatancyAngleScalingFactor();
}

void UAGX_TerrainMaterial::CopyTerrainMaterialProperties(const UAGX_TerrainMaterial* Source)
{
	if (Source)
	{
		// As of now, this property is not used for terrain (replaced by the terrain specific bulk
		// properties) and will always have default values.
		Bulk = Source->Bulk;

		Surface = Source->Surface;
		TerrainBulk = Source->TerrainBulk;
		TerrainCompaction = Source->TerrainCompaction;
	}
}

UAGX_MaterialBase* UAGX_TerrainMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_TerrainMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TerrainMaterial::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_TerrainMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_TerrainMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Surface properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, bFrictionEnabled),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.bFrictionEnabled, SetFrictionEnabled) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Roughness),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.Roughness, SetRoughness) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Viscosity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.Viscosity, SetSurfaceViscosity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveForce),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Surface.AdhesiveForce = This->Surface.AdhesiveForce;
			}
			This->SetAdhesion(This->Surface.AdhesiveForce, This->Surface.AdhesiveOverlap);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveOverlap),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Surface.AdhesiveOverlap = This->Surface.AdhesiveOverlap;
			}
			This->SetAdhesion(This->Surface.AdhesiveForce, This->Surface.AdhesiveOverlap);
		});

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, AdhesionOverlapFactor),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainBulk.AdhesionOverlapFactor, SetAdhesionOverlapFactor)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Cohesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.Cohesion, SetCohesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Density),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.Density, SetDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, DilatancyAngle),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.DilatancyAngle, SetDilatancyAngle) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, FrictionAngle),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.FrictionAngle, SetFrictionAngle) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, MaxDensity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.MaxDensity, SetMaxDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, PoissonsRatio),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.PoissonsRatio, SetPoissonsRatio) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, SwellFactor),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.SwellFactor, SetSwellFactor) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, YoungsModulus),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.YoungsModulus, SetYoungsModulus) });

	// Compaction properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, AngleOfReposeCompactionRate),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.AngleOfReposeCompactionRate, SetAngleOfReposeCompactionRate)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, BankStatePhi0),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainCompaction.BankStatePhi0, SetBankStatePhi) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompactionTimeRelaxationConstant),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.CompactionTimeRelaxationConstant,
				SetCompactionTimeRelaxationConstant)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompressionIndex),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.CompressionIndex, SetCompressionIndex)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, HardeningConstantKe),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.HardeningConstantKe, SetHardeningConstantKe)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, HardeningConstantNe),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.HardeningConstantNe, SetHardeningConstantNe)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, PreconsolidationStress),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.PreconsolidationStress, SetPreconsolidationStress)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, StressCutOffFraction),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.StressCutOffFraction, SetStressCutOffFraction)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, DilatancyAngleScalingFactor),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.DilatancyAngleScalingFactor, SetDilatancyAngleScalingFactor)
		});
}
#endif

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetOrCreateTerrainMaterialNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateTerrainMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateTerrainMaterialNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasTerrainMaterialNative())
	{
		CreateTerrainMaterialNative(PlayingWorld);
	}
	return GetTerrainMaterialNative();
}

FShapeMaterialBarrier* UAGX_TerrainMaterial::GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateShapeMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasShapeMaterialNative())
	{
		CreateShapeMaterialNative(PlayingWorld);
	}
	return GetShapeMaterialNative();
}

UAGX_TerrainMaterial* UAGX_TerrainMaterial::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainMaterial* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TerrainMaterial* NewInstance = NewObject<UAGX_TerrainMaterial>(
		Outer, UAGX_TerrainMaterial::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;

	// Copy the terrain material properties
	NewInstance->CopyTerrainMaterialProperties(Source);

	NewInstance->CreateTerrainMaterialNative(PlayingWorld);
	NewInstance->CreateShapeMaterialNative(PlayingWorld);

	return NewInstance;
}

void UAGX_TerrainMaterial::CreateTerrainMaterialNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateTerrainMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateTerrainMaterialNative(PlayingWorld);
		return;
	}

	AGX_CHECK(IsInstance());
	if (TerrainMaterialNativeBarrier.HasNative())
	{
		TerrainMaterialNativeBarrier.ReleaseNative();
	}

	TerrainMaterialNativeBarrier.AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasTerrainMaterialNative());

	UpdateTerrainMaterialNativeProperties();
}

void UAGX_TerrainMaterial::CreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateShapeMaterialNative was called on UAGX_TerrainMaterial '%s' "
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateShapeMaterialNative(PlayingWorld);
		return;
	}

	AGX_CHECK(IsInstance());
	if (ShapeMaterialNativeBarrier.HasNative())
	{
		ShapeMaterialNativeBarrier.ReleaseNative();
	}

	ShapeMaterialNativeBarrier.AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasShapeMaterialNative());

	UpdateShapeMaterialNativeProperties();
}

bool UAGX_TerrainMaterial::HasTerrainMaterialNative() const
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return false;
		}

		return Instance->HasTerrainMaterialNative();
	}

	AGX_CHECK(IsInstance());
	return TerrainMaterialNativeBarrier.HasNative();
}

bool UAGX_TerrainMaterial::HasShapeMaterialNative() const
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return false;
		}

		return Instance->HasShapeMaterialNative();
	}

	AGX_CHECK(IsInstance());
	return ShapeMaterialNativeBarrier.HasNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetTerrainMaterialNative()
{
	return HasTerrainMaterialNative() ? &TerrainMaterialNativeBarrier : nullptr;
}

FShapeMaterialBarrier* UAGX_TerrainMaterial::GetShapeMaterialNative()
{
	return HasShapeMaterialNative() ? &ShapeMaterialNativeBarrier : nullptr;
}

void UAGX_TerrainMaterial::UpdateTerrainMaterialNativeProperties()
{
	if (HasTerrainMaterialNative())
	{
		AGX_CHECK(IsInstance());
		TerrainMaterialNativeBarrier.SetName(TCHAR_TO_UTF8(*GetName()));

		// Set Bulk properties.
		TerrainMaterialNativeBarrier.SetAdhesionOverlapFactor(TerrainBulk.AdhesionOverlapFactor);
		TerrainMaterialNativeBarrier.SetCohesion(TerrainBulk.Cohesion);
		TerrainMaterialNativeBarrier.SetDensity(TerrainBulk.Density);
		TerrainMaterialNativeBarrier.SetDilatancyAngle(TerrainBulk.DilatancyAngle);
		TerrainMaterialNativeBarrier.SetFrictionAngle(TerrainBulk.FrictionAngle);
		TerrainMaterialNativeBarrier.SetMaxDensity(TerrainBulk.MaxDensity);
		TerrainMaterialNativeBarrier.SetPoissonsRatio(TerrainBulk.PoissonsRatio);
		TerrainMaterialNativeBarrier.SetSwellFactor(TerrainBulk.SwellFactor);
		TerrainMaterialNativeBarrier.SetYoungsModulus(TerrainBulk.YoungsModulus);

		// Set Compaction properties.
		TerrainMaterialNativeBarrier.SetAngleOfReposeCompactionRate(
			TerrainCompaction.AngleOfReposeCompactionRate);
		TerrainMaterialNativeBarrier.SetBankStatePhi(TerrainCompaction.BankStatePhi0);
		TerrainMaterialNativeBarrier.SetCompactionTimeRelaxationConstant(
			TerrainCompaction.CompactionTimeRelaxationConstant);
		TerrainMaterialNativeBarrier.SetCompressionIndex(TerrainCompaction.CompressionIndex);
		TerrainMaterialNativeBarrier.SetHardeningConstantKe(TerrainCompaction.HardeningConstantKe);
		TerrainMaterialNativeBarrier.SetHardeningConstantNe(TerrainCompaction.HardeningConstantNe);
		TerrainMaterialNativeBarrier.SetPreconsolidationStress(
			TerrainCompaction.PreconsolidationStress);
		TerrainMaterialNativeBarrier.SetStressCutOffFraction(
			TerrainCompaction.StressCutOffFraction);
		TerrainMaterialNativeBarrier.SetDilatancyAngleScalingFactor(
			TerrainCompaction.DilatancyAngleScalingFactor);
	}
}

void UAGX_TerrainMaterial::UpdateShapeMaterialNativeProperties()
{
	if (HasShapeMaterialNative())
	{
		AGX_CHECK(IsInstance());
		ShapeMaterialNativeBarrier.SetName(TCHAR_TO_UTF8(*GetName()));

		ShapeMaterialNativeBarrier.SetDensity(Bulk.Density);
		ShapeMaterialNativeBarrier.SetYoungsModulus(Bulk.YoungsModulus);
		ShapeMaterialNativeBarrier.SetBulkViscosity(Bulk.Viscosity);
		ShapeMaterialNativeBarrier.SetSpookDamping(Bulk.SpookDamping);
		ShapeMaterialNativeBarrier.SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		ShapeMaterialNativeBarrier.SetFrictionEnabled(Surface.bFrictionEnabled);
		ShapeMaterialNativeBarrier.SetRoughness(Surface.Roughness);
		ShapeMaterialNativeBarrier.SetSurfaceViscosity(Surface.Viscosity);
		ShapeMaterialNativeBarrier.SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
	}
}

bool UAGX_TerrainMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset, if the below check
	// has never failed.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}
