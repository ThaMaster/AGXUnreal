// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterial.h"

// AGX Dynamics for Unreal includes.
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
	if (Instance != nullptr)
	{
		Instance->SetFrictionEnabled(Enabled);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.bFrictionEnabled = Enabled;
	}
}

bool UAGX_TerrainMaterial::GetFrictionEnabled() const
{
	if (Instance != nullptr)
	{
		return Instance->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_TerrainMaterial::SetRoughness(float Roughness)
{
	if (Instance != nullptr)
	{
		Instance->SetRoughness(Roughness);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.Roughness = Roughness;
	}
}

float UAGX_TerrainMaterial::GetRoughness() const
{
	if (Instance != nullptr)
	{
		return Instance->GetRoughness();
	}

	return Surface.Roughness;
}

void UAGX_TerrainMaterial::SetSurfaceViscosity(float Viscosity)
{
	if (Instance != nullptr)
	{
		Instance->SetSurfaceViscosity(Viscosity);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.Viscosity = Viscosity;
	}
}

float UAGX_TerrainMaterial::GetSurfaceViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSurfaceViscosity();
	}

	return Surface.Viscosity;
}

void UAGX_TerrainMaterial::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	if (Instance != nullptr)
	{
		Instance->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
	}
}

float UAGX_TerrainMaterial::GetAdhesiveForce() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveForce();
	}

	return Surface.AdhesiveForce;
}

float UAGX_TerrainMaterial::GetAdhesiveOverlap() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveOverlap();
	}

	return Surface.AdhesiveOverlap;
}

// Bulk properties.
void UAGX_TerrainMaterial::SetAdhesionOverlapFactor(float AdhesionOverlapFactor)
{
	TerrainBulk.AdhesionOverlapFactor = static_cast<double>(AdhesionOverlapFactor);
}

float UAGX_TerrainMaterial::GetAdhesionOverlapFactor() const
{
	return static_cast<float>(TerrainBulk.AdhesionOverlapFactor);
}

void UAGX_TerrainMaterial::SetCohesion(float Cohesion)
{
	TerrainBulk.Cohesion = static_cast<double>(Cohesion);
}

float UAGX_TerrainMaterial::GetCohesion() const
{
	return static_cast<float>(TerrainBulk.Cohesion);
}

void UAGX_TerrainMaterial::SetDensity(float Density)
{
	TerrainBulk.Density = static_cast<double>(Density);
}

float UAGX_TerrainMaterial::GetDensity() const
{
	return static_cast<float>(TerrainBulk.Density);
}

void UAGX_TerrainMaterial::SetDilatancyAngle(float DilatancyAngle)
{
	TerrainBulk.DilatancyAngle = static_cast<double>(DilatancyAngle);
}

float UAGX_TerrainMaterial::GetDilatancyAngle() const
{
	return static_cast<float>(TerrainBulk.DilatancyAngle);
}

void UAGX_TerrainMaterial::SetFrictionAngle(float FrictionAngle)
{
	TerrainBulk.FrictionAngle = static_cast<double>(FrictionAngle);
}

float UAGX_TerrainMaterial::GetFrictionAngle() const
{
	return static_cast<float>(TerrainBulk.FrictionAngle);
}

void UAGX_TerrainMaterial::SetMaxDensity(float MaxDensity)
{
	TerrainBulk.MaxDensity = static_cast<double>(MaxDensity);
}

float UAGX_TerrainMaterial::GetMaxDensity() const
{
	return static_cast<float>(TerrainBulk.MaxDensity);
}

void UAGX_TerrainMaterial::SetPoissonsRatio(float PoissonsRatio)
{
	TerrainBulk.PoissonsRatio = static_cast<double>(PoissonsRatio);
}

float UAGX_TerrainMaterial::GetPoissonsRatio() const
{
	return static_cast<float>(TerrainBulk.PoissonsRatio);
}

void UAGX_TerrainMaterial::SetSwellFactor(float SwellFactor)
{
	TerrainBulk.SwellFactor = static_cast<double>(SwellFactor);
}

float UAGX_TerrainMaterial::GetSwellFactor() const
{
	return static_cast<float>(TerrainBulk.SwellFactor);
}

void UAGX_TerrainMaterial::SetYoungsModulus(float YoungsModulus)
{
	TerrainBulk.YoungsModulus = static_cast<double>(YoungsModulus);
}

float UAGX_TerrainMaterial::GetYoungsModulus() const
{
	return static_cast<float>(TerrainBulk.YoungsModulus);
}

// Compaction properties.
void UAGX_TerrainMaterial::SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate)
{
	TerrainCompaction.AngleOfReposeCompactionRate =
		static_cast<double>(AngleOfReposeCompactionRate);
}

float UAGX_TerrainMaterial::GetAngleOfReposeCompactionRate() const
{
	return static_cast<float>(TerrainCompaction.AngleOfReposeCompactionRate);
}

void UAGX_TerrainMaterial::SetBankStatePhi(float Phi0)
{
	TerrainCompaction.Phi0 = static_cast<double>(Phi0);
}

float UAGX_TerrainMaterial::GetBankStatePhi() const
{
	return static_cast<float>(TerrainCompaction.Phi0);
}

void UAGX_TerrainMaterial::SetCompactionTimeRelaxationConstant(
	float CompactionTimeRelaxationConstant)
{
	TerrainCompaction.CompactionTimeRelaxationConstant =
		static_cast<double>(CompactionTimeRelaxationConstant);
}

float UAGX_TerrainMaterial::GetCompactionTimeRelaxationConstant() const
{
	return static_cast<float>(TerrainCompaction.CompactionTimeRelaxationConstant);
}

void UAGX_TerrainMaterial::SetCompressionIndex(float CompressionIndex)
{
	TerrainCompaction.CompressionIndex = static_cast<double>(CompressionIndex);
}

float UAGX_TerrainMaterial::GetCompressionIndex() const
{
	return static_cast<float>(TerrainCompaction.CompressionIndex);
}

void UAGX_TerrainMaterial::SetHardeningConstantKe(float K_e)
{
	TerrainCompaction.K_e = static_cast<double>(K_e);
}

float UAGX_TerrainMaterial::GetHardeningConstantKe() const
{
	return static_cast<float>(TerrainCompaction.K_e);
}

void UAGX_TerrainMaterial::SetHardeningConstantNe(float N_e)
{
	TerrainCompaction.N_e = static_cast<double>(N_e);
}

float UAGX_TerrainMaterial::GetHardeningConstantNe() const
{
	return static_cast<float>(TerrainCompaction.N_e);
}

void UAGX_TerrainMaterial::SetPreconsolidationStress(float PreconsolidationStress)
{
	TerrainCompaction.PreconsolidationStress = static_cast<double>(PreconsolidationStress);
}

float UAGX_TerrainMaterial::GetPreconsolidationStress() const
{
	return static_cast<float>(TerrainCompaction.PreconsolidationStress);
}

void UAGX_TerrainMaterial::SetStressCutOffFraction(float StressCutOffFraction)
{
	TerrainCompaction.StressCutOffFraction = static_cast<double>(StressCutOffFraction);
}

float UAGX_TerrainMaterial::GetStressCutOffFraction() const
{
	return static_cast<float>(TerrainCompaction.StressCutOffFraction);
}

void UAGX_TerrainMaterial::CopyFrom(const FTerrainMaterialBarrier& Source)
{
	TerrainBulk = FAGX_TerrainBulkProperties();
	TerrainBulk.AdhesionOverlapFactor = Source.GetAdhesionOverlapFactor();
	TerrainBulk.Cohesion = Source.GetCohesion();
	TerrainBulk.Density = Source.GetDensity();
	TerrainBulk.DilatancyAngle = Source.GetDilatancyAngle();
	TerrainBulk.FrictionAngle = Source.GetFrictionAngle();
	TerrainBulk.MaxDensity = Source.GetMaximumDensity();
	TerrainBulk.PoissonsRatio = Source.GetPoissonsRatio();
	TerrainBulk.SwellFactor = Source.GetSwellFactor();
	TerrainBulk.YoungsModulus = Source.GetYoungsModulus();

	TerrainCompaction = FAGX_TerrainCompactionProperties();
	TerrainCompaction.AngleOfReposeCompactionRate = Source.GetAngleOfReposeCompactionRate();
	TerrainCompaction.Phi0 = Source.GetBankStatePhi();
	TerrainCompaction.CompactionTimeRelaxationConstant =
		Source.GetCompactionTimeRelaxationConstant();
	TerrainCompaction.CompressionIndex = Source.GetCompressionIndex();
	TerrainCompaction.K_e = Source.GetHardeningConstantKE();
	TerrainCompaction.N_e = Source.GetHardeningConstantNE();
	TerrainCompaction.PreconsolidationStress = Source.GetPreconsolidationStress();
	TerrainCompaction.StressCutOffFraction = Source.GetStressCutOffFraction();
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

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveOverlap),
		[](ThisClass* This)
		{
			This->SetAdhesion(
				static_cast<float>(This->Surface.AdhesiveForce),
				static_cast<float>(This->Surface.AdhesiveOverlap));
		});

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, AdhesionOverlapFactor),
		[](ThisClass* This) {
			This->SetAdhesionOverlapFactor(
				static_cast<float>(This->TerrainBulk.AdhesionOverlapFactor));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Cohesion),
		[](ThisClass* This) { This->SetCohesion(static_cast<float>(This->TerrainBulk.Cohesion)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Density),
		[](ThisClass* This) { This->SetDensity(static_cast<float>(This->TerrainBulk.Density)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, DilatancyAngle),
		[](ThisClass* This)
		{ This->SetDilatancyAngle(static_cast<float>(This->TerrainBulk.DilatancyAngle)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, FrictionAngle),
		[](ThisClass* This)
		{ This->SetFrictionAngle(static_cast<float>(This->TerrainBulk.FrictionAngle)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, MaxDensity),
		[](ThisClass* This)
		{ This->SetMaxDensity(static_cast<float>(This->TerrainBulk.MaxDensity)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, PoissonsRatio),
		[](ThisClass* This)
		{ This->SetPoissonsRatio(static_cast<float>(This->TerrainBulk.PoissonsRatio)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, SwellFactor),
		[](ThisClass* This)
		{ This->SetSwellFactor(static_cast<float>(This->TerrainBulk.SwellFactor)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, YoungsModulus),
		[](ThisClass* This)
		{ This->SetYoungsModulus(static_cast<float>(This->TerrainBulk.YoungsModulus)); });

	// Compaction properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, AngleOfReposeCompactionRate),
		[](ThisClass* This)
		{
			This->SetAngleOfReposeCompactionRate(
				static_cast<float>(This->TerrainCompaction.AngleOfReposeCompactionRate));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, Phi0),
		[](ThisClass* This)
		{ This->SetBankStatePhi(static_cast<float>(This->TerrainCompaction.Phi0)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompactionTimeRelaxationConstant),
		[](ThisClass* This)
		{
			This->SetCompactionTimeRelaxationConstant(
				static_cast<float>(This->TerrainCompaction.CompactionTimeRelaxationConstant));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompressionIndex),
		[](ThisClass* This) {
			This->SetCompressionIndex(static_cast<float>(This->TerrainCompaction.CompressionIndex));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, K_e),
		[](ThisClass* This)
		{ This->SetHardeningConstantKe(static_cast<float>(This->TerrainCompaction.K_e)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, N_e),
		[](ThisClass* This)
		{ This->SetHardeningConstantNe(static_cast<float>(This->TerrainCompaction.N_e)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, PreconsolidationStress),
		[](ThisClass* This)
		{
			This->SetPreconsolidationStress(
				static_cast<float>(This->TerrainCompaction.PreconsolidationStress));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, StressCutOffFraction),
		[](ThisClass* This)
		{
			This->SetStressCutOffFraction(
				static_cast<float>(This->TerrainCompaction.StressCutOffFraction));
		});
}
#endif

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetOrCreateTerrainMaterialNative(
	UWorld* PlayingWorld)
{
	if (IsAsset())
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

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
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
	if (IsAsset())
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
	check(Source->IsAsset());
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
	if (IsAsset())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateTerrainMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."), *GetName());
			return nullptr;
		}

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
	}

	TerrainMaterialNativeBarrier.Reset(new FTerrainMaterialBarrier());
	TerrainMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasTerrainMaterialNative());

	UpdateTerrainMaterialNativeProperties();
}

void UAGX_TerrainMaterial::CreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (IsAsset())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateShapeMaterialNative was called on UAGX_TerrainMaterial '%s' "
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."), *GetName());
			return nullptr;
		}

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
	}

	ShapeMaterialNativeBarrier.Reset(new FShapeMaterialBarrier());
	ShapeMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasShapeMaterialNative());

	UpdateShapeMaterialNativeProperties();
}

bool UAGX_TerrainMaterial::HasTerrainMaterialNative() const
{
	return TerrainMaterialNativeBarrier && TerrainMaterialNativeBarrier->HasNative();
}

bool UAGX_TerrainMaterial::HasShapeMaterialNative() const
{
	return ShapeMaterialNativeBarrier && ShapeMaterialNativeBarrier->HasNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetTerrainMaterialNative()
{
	return TerrainMaterialNativeBarrier.Get();
}

FShapeMaterialBarrier* UAGX_TerrainMaterial::GetShapeMaterialNative()
{
	return ShapeMaterialNativeBarrier.Get();
}

void UAGX_TerrainMaterial::UpdateTerrainMaterialNativeProperties()
{
	if (HasTerrainMaterialNative())
	{
		AGX_CHECK(IsInstance());
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

void UAGX_TerrainMaterial::UpdateShapeMaterialNativeProperties()
{
	if (HasShapeMaterialNative())
	{
		AGX_CHECK(IsInstance());
		ShapeMaterialNativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		ShapeMaterialNativeBarrier->SetDensity(Bulk.Density);
		ShapeMaterialNativeBarrier->SetYoungsModulus(Bulk.YoungsModulus);
		ShapeMaterialNativeBarrier->SetBulkViscosity(Bulk.Viscosity);
		ShapeMaterialNativeBarrier->SetSpookDamping(Bulk.SpookDamping);
		ShapeMaterialNativeBarrier->SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		ShapeMaterialNativeBarrier->SetFrictionEnabled(Surface.bFrictionEnabled);
		ShapeMaterialNativeBarrier->SetRoughness(Surface.Roughness);
		ShapeMaterialNativeBarrier->SetSurfaceViscosity(Surface.Viscosity);
		ShapeMaterialNativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
	}
}

bool UAGX_TerrainMaterial::IsAsset() const
{
	return !IsInstance();
}

bool UAGX_TerrainMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	return Asset != nullptr;
}