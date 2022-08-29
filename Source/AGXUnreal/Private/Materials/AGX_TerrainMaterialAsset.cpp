// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_TerrainMaterialInstance.h"

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
void UAGX_TerrainMaterialAsset::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainMaterialAsset::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_TerrainMaterialAsset::InitPropertyDispatcher()
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
		[](ThisClass* This) { This->SetFrictionEnabled(This->Surface.bFrictionEnabled); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Roughness),
		[](ThisClass* This) { This->SetRoughness(This->Surface.Roughness); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Viscosity),
		[](ThisClass* This) { This->SetSurfaceViscosity(This->Surface.Viscosity); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveForce),
		[](ThisClass* This)
		{
			This->SetAdhesion(
				static_cast<float>(This->Surface.AdhesiveForce),
				static_cast<float>(This->Surface.AdhesiveOverlap));
		});

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, AdhesionOverlapFactor),
		[](ThisClass* This) {
			This->SetAdhesionOverlapFactor(
				static_cast<float>(This->TerrainBulk.AdhesionOverlapFactor));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Cohesion),
		[](ThisClass* This) { This->SetCohesion(static_cast<float>(This->TerrainBulk.Cohesion)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Density),
		[](ThisClass* This) { This->SetDensity(static_cast<float>(This->TerrainBulk.Density)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, DilatancyAngle),
		[](ThisClass* This)
		{ This->SetDilatancyAngle(static_cast<float>(This->TerrainBulk.DilatancyAngle)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, FrictionAngle),
		[](ThisClass* This)
		{ This->SetFrictionAngle(static_cast<float>(This->TerrainBulk.FrictionAngle)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, MaxDensity),
		[](ThisClass* This)
		{ This->SetMaxDensity(static_cast<float>(This->TerrainBulk.MaxDensity)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, PoissonsRatio),
		[](ThisClass* This)
		{ This->SetPoissonsRatio(static_cast<float>(This->TerrainBulk.PoissonsRatio)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, SwellFactor),
		[](ThisClass* This)
		{ This->SetSwellFactor(static_cast<float>(This->TerrainBulk.SwellFactor)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, YoungsModulus),
		[](ThisClass* This)
		{ This->SetYoungsModulus(static_cast<float>(This->TerrainBulk.YoungsModulus)); });

	// Compaction properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, AngleOfReposeCompactionRate),
		[](ThisClass* This)
		{
			This->SetAngleOfReposeCompactionRate(
				static_cast<float>(This->TerrainCompaction.AngleOfReposeCompactionRate));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, Phi0),
		[](ThisClass* This)
		{ This->SetBankStatePhi(static_cast<float>(This->TerrainCompaction.Phi0)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompactionTimeRelaxationConstant),
		[](ThisClass* This)
		{
			This->SetCompactionTimeRelaxationConstant(
				static_cast<float>(This->TerrainCompaction.CompactionTimeRelaxationConstant));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompressionIndex),
		[](ThisClass* This)
		{
			This->SetCompressionIndex(static_cast<float>(This->TerrainCompaction.CompressionIndex));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, K_e),
		[](ThisClass* This)
		{ This->SetHardeningConstantKe(static_cast<float>(This->TerrainCompaction.K_e)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, N_e),
		[](ThisClass* This)
		{ This->SetHardeningConstantNe(static_cast<float>(This->TerrainCompaction.N_e)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, PreconsolidationStress),
		[](ThisClass* This)
		{
			This->SetPreconsolidationStress(
				static_cast<float>(This->TerrainCompaction.PreconsolidationStress));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterialBase, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, StressCutOffFraction),
		[](ThisClass* This)
		{
			This->SetStressCutOffFraction(
				static_cast<float>(This->TerrainCompaction.StressCutOffFraction));
		});
}
#endif

// Surface properties.
void UAGX_TerrainMaterialAsset::SetFrictionEnabled(bool Enabled)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetFrictionEnabled(Enabled);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.bFrictionEnabled = Enabled;
	}
}

bool UAGX_TerrainMaterialAsset::GetFrictionEnabled() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_TerrainMaterialAsset::SetRoughness(float Roughness)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetRoughness(Roughness);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.Roughness = Roughness;
	}
}

float UAGX_TerrainMaterialAsset::GetRoughness() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetRoughness();
	}

	return Surface.Roughness;
}

void UAGX_TerrainMaterialAsset::SetSurfaceViscosity(float Viscosity)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetSurfaceViscosity(Viscosity);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.Viscosity = Viscosity;
	}
}

float UAGX_TerrainMaterialAsset::GetSurfaceViscosity() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetSurfaceViscosity();
	}

	return Surface.Viscosity;
}

void UAGX_TerrainMaterialAsset::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
	}
}

float UAGX_TerrainMaterialAsset::GetAdhesiveForce() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetAdhesiveForce();
	}

	return Surface.AdhesiveForce;
}

float UAGX_TerrainMaterialAsset::GetAdhesiveOverlap() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetAdhesiveOverlap();
	}

	return Surface.AdhesiveOverlap;
}

// Bulk properties.
void UAGX_TerrainMaterialAsset::SetAdhesionOverlapFactor(float AdhesionOverlapFactor)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetAdhesionOverlapFactor(AdhesionOverlapFactor);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.AdhesionOverlapFactor = AdhesionOverlapFactor;
	}
}

float UAGX_TerrainMaterialAsset::GetAdhesionOverlapFactor() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetAdhesionOverlapFactor();
	}

	return TerrainBulk.AdhesionOverlapFactor;
}

void UAGX_TerrainMaterialAsset::SetCohesion(float Cohesion)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetCohesion(Cohesion);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.Cohesion = Cohesion;
	}
}

float UAGX_TerrainMaterialAsset::GetCohesion() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetCohesion();
	}

	return TerrainBulk.Cohesion;
}

void UAGX_TerrainMaterialAsset::SetDensity(float Density)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetDensity(Density);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.Density = Density;
	}
}

float UAGX_TerrainMaterialAsset::GetDensity() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetDensity();
	}

	return TerrainBulk.Density;
}

void UAGX_TerrainMaterialAsset::SetDilatancyAngle(float DilatancyAngle)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetDilatancyAngle(DilatancyAngle);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.DilatancyAngle = DilatancyAngle;
	}
}

float UAGX_TerrainMaterialAsset::GetDilatancyAngle() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetDilatancyAngle();
	}

	return TerrainBulk.DilatancyAngle;
}

void UAGX_TerrainMaterialAsset::SetFrictionAngle(float FrictionAngle)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetFrictionAngle(FrictionAngle);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.FrictionAngle = FrictionAngle;
	}
}

float UAGX_TerrainMaterialAsset::GetFrictionAngle() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetFrictionAngle();
	}

	return TerrainBulk.FrictionAngle;
}

void UAGX_TerrainMaterialAsset::SetMaxDensity(float MaxDensity)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetMaxDensity(MaxDensity);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.MaxDensity = MaxDensity;
	}
}

float UAGX_TerrainMaterialAsset::GetMaxDensity() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetMaxDensity();
	}

	return TerrainBulk.MaxDensity;
}

void UAGX_TerrainMaterialAsset::SetPoissonsRatio(float PoissonsRatio)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetPoissonsRatio(PoissonsRatio);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.PoissonsRatio = PoissonsRatio;
	}
}

float UAGX_TerrainMaterialAsset::GetPoissonsRatio() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetPoissonsRatio();
	}

	return TerrainBulk.PoissonsRatio;
}

void UAGX_TerrainMaterialAsset::SetSwellFactor(float SwellFactor)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetSwellFactor(SwellFactor);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.SwellFactor = SwellFactor;
	}
}

float UAGX_TerrainMaterialAsset::GetSwellFactor() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetSwellFactor();
	}

	return TerrainBulk.SwellFactor;
}

void UAGX_TerrainMaterialAsset::SetYoungsModulus(float YoungsModulus)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetYoungsModulus(YoungsModulus);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainBulk.YoungsModulus = YoungsModulus;
	}
}

float UAGX_TerrainMaterialAsset::GetYoungsModulus() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetYoungsModulus();
	}

	return TerrainBulk.YoungsModulus;
}

// Compaction properties.
void UAGX_TerrainMaterialAsset::SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetAngleOfReposeCompactionRate(AngleOfReposeCompactionRate);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.AngleOfReposeCompactionRate = AngleOfReposeCompactionRate;
	}
}

float UAGX_TerrainMaterialAsset::GetAngleOfReposeCompactionRate() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetAngleOfReposeCompactionRate();
	}

	return TerrainCompaction.AngleOfReposeCompactionRate;
}

void UAGX_TerrainMaterialAsset::SetBankStatePhi(float Phi0)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetBankStatePhi(Phi0);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.Phi0 = Phi0;
	}
}

float UAGX_TerrainMaterialAsset::GetBankStatePhi() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetBankStatePhi();
	}

	return TerrainCompaction.Phi0;
}

void UAGX_TerrainMaterialAsset::SetCompactionTimeRelaxationConstant(
	float CompactionTimeRelaxationConstant)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetCompactionTimeRelaxationConstant(
			CompactionTimeRelaxationConstant);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.CompactionTimeRelaxationConstant = CompactionTimeRelaxationConstant;
	}
}

float UAGX_TerrainMaterialAsset::GetCompactionTimeRelaxationConstant() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetCompactionTimeRelaxationConstant();
	}

	return TerrainCompaction.CompactionTimeRelaxationConstant;
}

void UAGX_TerrainMaterialAsset::SetCompressionIndex(float CompressionIndex)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetCompressionIndex(CompressionIndex);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.CompressionIndex = CompressionIndex;
	}
}

float UAGX_TerrainMaterialAsset::GetCompressionIndex() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetCompressionIndex();
	}

	return TerrainCompaction.CompressionIndex;
}

void UAGX_TerrainMaterialAsset::SetHardeningConstantKe(float K_e)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetHardeningConstantKe(K_e);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.K_e = K_e;
	}
}

float UAGX_TerrainMaterialAsset::GetHardeningConstantKe() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetHardeningConstantKe();
	}

	return TerrainCompaction.K_e;
}

void UAGX_TerrainMaterialAsset::SetHardeningConstantNe(float N_e)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetHardeningConstantNe(N_e);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.N_e = N_e;
	}
}

float UAGX_TerrainMaterialAsset::GetHardeningConstantNe() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetHardeningConstantNe();
	}

	return TerrainCompaction.N_e;
}

void UAGX_TerrainMaterialAsset::SetPreconsolidationStress(float PreconsolidationStress)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetPreconsolidationStress(PreconsolidationStress);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.PreconsolidationStress = PreconsolidationStress;
	}
}

float UAGX_TerrainMaterialAsset::GetPreconsolidationStress() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetPreconsolidationStress();
	}

	return TerrainCompaction.PreconsolidationStress;
}

void UAGX_TerrainMaterialAsset::SetStressCutOffFraction(float StressCutOffFraction)
{
	if (TerrainMaterialInstance != nullptr)
	{
		TerrainMaterialInstance->SetStressCutOffFraction(StressCutOffFraction);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		TerrainCompaction.StressCutOffFraction = StressCutOffFraction;
	}
}

float UAGX_TerrainMaterialAsset::GetStressCutOffFraction() const
{
	if (TerrainMaterialInstance != nullptr)
	{
		return TerrainMaterialInstance->GetStressCutOffFraction();
	}

	return TerrainCompaction.StressCutOffFraction;
}
