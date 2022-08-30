// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_ShapeMaterialInstance.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_MaterialBase* UAGX_ShapeMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ShapeMaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeMaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR

void UAGX_ShapeMaterialAsset::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeMaterialAsset::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeMaterialAsset::InitPropertyDispatcher()
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
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Density),
		[](ThisClass* This) { This->SetDensity(static_cast<float>(This->Bulk.Density)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Viscosity),
		[](ThisClass* This) { This->SetBulkViscosity(static_cast<float>(This->Bulk.Viscosity)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, SpookDamping),
		[](ThisClass* This)
		{ This->SetSpookDamping(static_cast<float>(This->Bulk.SpookDamping)); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MinElasticRestLength),
		[](ThisClass* This)
		{
			This->SetMinMaxElasticRestLength(
				static_cast<float>(This->Bulk.MinElasticRestLength),
				static_cast<float>(This->Bulk.MaxElasticRestLength));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MaxElasticRestLength),
		[](ThisClass* This)
		{
			This->SetMinMaxElasticRestLength(
				static_cast<float>(This->Bulk.MinElasticRestLength),
				static_cast<float>(This->Bulk.MaxElasticRestLength));
		});

	// Wire properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusStretch),
		[](ThisClass* This) { This->SetYoungsModulusStretch(This->Wire.YoungsModulusStretch); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusBend),
		[](ThisClass* This) { This->SetYoungsModulusBend(This->Wire.YoungsModulusBend); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingStretch),
		[](ThisClass* This) { This->SetSpookDampingStretch(This->Wire.SpookDampingStretch); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingBend),
		[](ThisClass* This) { This->SetSpookDampingBend(This->Wire.SpookDampingBend); });
}
#endif

void UAGX_ShapeMaterialAsset::SetDensity(float InDensity)
{
	if (Instance != nullptr)
	{
		Instance->SetDensity(InDensity);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Bulk.Density = InDensity;
	}
}

float UAGX_ShapeMaterialAsset::GetDensity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetDensity();
	}

	return Bulk.Density;
}

void UAGX_ShapeMaterialAsset::SetYoungsModulus(float InYoungsModulus)
{
	if (Instance != nullptr)
	{
		Instance->SetYoungsModulus(InYoungsModulus);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Bulk.YoungsModulus = InYoungsModulus;
	}
}

float UAGX_ShapeMaterialAsset::GetYoungsModulus() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulus();
	}

	return Bulk.YoungsModulus;
}

void UAGX_ShapeMaterialAsset::SetBulkViscosity(float InBulkViscosity)
{
	if (Instance != nullptr)
	{
		Instance->SetBulkViscosity(InBulkViscosity);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Bulk.Viscosity = InBulkViscosity;
	}
}

float UAGX_ShapeMaterialAsset::GetBulkViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetBulkViscosity();
	}

	return Bulk.Viscosity;
}

void UAGX_ShapeMaterialAsset::SetSpookDamping(float InSpookDamping)
{
	if (Instance != nullptr)
	{
		Instance->SetSpookDamping(InSpookDamping);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Bulk.SpookDamping = InSpookDamping;
	}
}

float UAGX_ShapeMaterialAsset::GetSpookDamping() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDamping();
	}

	return Bulk.SpookDamping;
}

void UAGX_ShapeMaterialAsset::SetMinMaxElasticRestLength(float InMin, float InMax)
{
	if (Instance != nullptr)
	{
		Instance->SetMinMaxElasticRestLength(InMin, InMax);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Bulk.MinElasticRestLength = InMin;
		Bulk.MaxElasticRestLength = InMax;
	}
}

float UAGX_ShapeMaterialAsset::GetMinElasticRestLength() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMinElasticRestLength();
	}

	return Bulk.MinElasticRestLength;
}

float UAGX_ShapeMaterialAsset::GetMaxElasticRestLength() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxElasticRestLength();
	}

	return Bulk.MaxElasticRestLength;
}

void UAGX_ShapeMaterialAsset::SetFrictionEnabled(bool Enabled)
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

bool UAGX_ShapeMaterialAsset::GetFrictionEnabled() const
{
	if (Instance != nullptr)
	{
		return Instance->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_ShapeMaterialAsset::SetRoughness(float Roughness)
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

float UAGX_ShapeMaterialAsset::GetRoughness() const
{
	if (Instance != nullptr)
	{
		return Instance->GetRoughness();
	}

	return Surface.Roughness;
}

void UAGX_ShapeMaterialAsset::SetSurfaceViscosity(float Viscosity)
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

float UAGX_ShapeMaterialAsset::GetSurfaceViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSurfaceViscosity();
	}

	return Surface.Viscosity;
}

void UAGX_ShapeMaterialAsset::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
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

float UAGX_ShapeMaterialAsset::GetAdhesiveForce() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveForce();
	}

	return Surface.AdhesiveForce;
}

float UAGX_ShapeMaterialAsset::GetAdhesiveOverlap() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveOverlap();
	}

	return Surface.AdhesiveOverlap;
}

float UAGX_ShapeMaterialAsset::GetYoungsModulusStretch() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulusStretch();
	}
	return Wire.YoungsModulusStretch;
}

void UAGX_ShapeMaterialAsset::SetYoungsModulusStretch(float InYoungsModulus)
{
	if (Instance != nullptr)
	{
		Instance->SetYoungsModulusStretch(InYoungsModulus);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Wire.YoungsModulusStretch = InYoungsModulus;
	}
}

float UAGX_ShapeMaterialAsset::GetYoungsModulusBend() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulusBend();
	}
	return Wire.YoungsModulusBend;
}

void UAGX_ShapeMaterialAsset::SetYoungsModulusBend(float InYoungsModulus)
{
	if (Instance != nullptr)
	{
		Instance->SetYoungsModulusBend(InYoungsModulus);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Wire.YoungsModulusBend = InYoungsModulus;
	}
}

float UAGX_ShapeMaterialAsset::GetSpookDampingStretch() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDampingStretch();
	}
	return Wire.SpookDampingStretch;
}

void UAGX_ShapeMaterialAsset::SetSpookDampingStretch(float InSpookDamping)
{
	if (Instance != nullptr)
	{
		Instance->SetSpookDampingStretch(InSpookDamping);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Wire.SpookDampingStretch = InSpookDamping;
	}
}

float UAGX_ShapeMaterialAsset::GetSpookDampingBend() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDampingBend();
	}
	return Wire.SpookDampingBend;
}

void UAGX_ShapeMaterialAsset::SetSpookDampingBend(float InSpookDamping)
{
	if (Instance != nullptr)
	{
		Instance->SetSpookDampingBend(InSpookDamping);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing directly to this asset.
		Wire.SpookDampingBend = InSpookDamping;
	}
}
