// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"

namespace AGX_ShapeMaterial_helpers
{
	void LogBadUsage(
		const FString& FunctionName, const FString& ActualInstanceOrAsset,
		const FString& AttemptedInstanceOrAsset)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Function '%s' was called on an %s of class UAGX_ShapeMaterial "
				 "which is not allowed. This function may only be called on an %s of this class."),
			*FunctionName, *ActualInstanceOrAsset, *AttemptedInstanceOrAsset);
	}
}

#if WITH_EDITOR
void UAGX_ShapeMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeMaterial::InitPropertyDispatcher()
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
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Surface.Roughness, SetRoughness) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Viscosity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Surface.Viscosity, SetSurfaceViscosity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveForce),
		[](ThisClass* This)
		{
			if (This->IsInstanceAGX())
			{
				This->Asset->Surface.AdhesiveForce = This->Surface.AdhesiveForce;
			}
			This->SetAdhesion(
				static_cast<float>(This->Surface.AdhesiveForce),
				static_cast<float>(This->Surface.AdhesiveOverlap));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveOverlap),
		[](ThisClass* This)
		{
			if (This->IsInstanceAGX())
			{
				This->Asset->Surface.AdhesiveOverlap = This->Surface.AdhesiveOverlap;
			}
			This->SetAdhesion(
				static_cast<float>(This->Surface.AdhesiveForce),
				static_cast<float>(This->Surface.AdhesiveOverlap));
		});

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Density),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Bulk.Density, SetDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Viscosity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Bulk.Viscosity, SetBulkViscosity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, SpookDamping),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Bulk.SpookDamping, SetSpookDamping) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MinElasticRestLength),
		[](ThisClass* This)
		{
			if (This->IsInstanceAGX())
			{
				This->Asset->Bulk.MinElasticRestLength = This->Bulk.MinElasticRestLength;
			}
			This->SetMinMaxElasticRestLength(
				static_cast<float>(This->Bulk.MinElasticRestLength),
				static_cast<float>(This->Bulk.MaxElasticRestLength));
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MaxElasticRestLength),
		[](ThisClass* This)
		{
			if (This->IsInstanceAGX())
			{
				This->Asset->Bulk.MaxElasticRestLength = This->Bulk.MaxElasticRestLength;
			}
			This->SetMinMaxElasticRestLength(
				static_cast<float>(This->Bulk.MinElasticRestLength),
				static_cast<float>(This->Bulk.MaxElasticRestLength));
		});

	// Wire properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusStretch),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Wire.YoungsModulusStretch, SetYoungsModulusStretch)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusBend),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Wire.YoungsModulusBend, SetYoungsModulusBend) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingStretch),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Wire.SpookDampingStretch, SetSpookDampingStretch) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingBend),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(Wire.SpookDampingBend, SetSpookDampingBend) });
}
#endif

// Surface properties.
void UAGX_ShapeMaterial::SetFrictionEnabled(bool Enabled)
{
	AGX_ASSET_SETTER_IMPL(Surface.bFrictionEnabled, Enabled, SetFrictionEnabled);
}

bool UAGX_ShapeMaterial::GetFrictionEnabled() const
{
	AGX_ASSET_GETTER_IMPL(Surface.bFrictionEnabled, GetFrictionEnabled);
}

void UAGX_ShapeMaterial::SetRoughness(float InRoughness)
{
	AGX_ASSET_SETTER_F2D_IMPL(Surface.Roughness, InRoughness, SetRoughness);
}

float UAGX_ShapeMaterial::GetRoughness() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Surface.Roughness, GetRoughness);
}

void UAGX_ShapeMaterial::SetSurfaceViscosity(float Viscosity)
{
	AGX_ASSET_SETTER_F2D_IMPL(Surface.Viscosity, Viscosity, SetSurfaceViscosity);
}

float UAGX_ShapeMaterial::GetSurfaceViscosity() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Surface.Viscosity, GetSurfaceViscosity);
}

void UAGX_ShapeMaterial::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	if (IsInstanceAGX())
	{
		Surface.AdhesiveForce = static_cast<double>(AdhesiveForce);
		Surface.AdhesiveOverlap = static_cast<double>(AdhesiveOverlap);
		if (HasNative())
		{
			NativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
		}
	}
	else // IsAssetAGX
	{
		if (Instance != nullptr)
		{
			Instance->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
			return;
		}
		Surface.AdhesiveForce = static_cast<double>(AdhesiveForce);
		Surface.AdhesiveOverlap = static_cast<double>(AdhesiveOverlap);
	}
}

float UAGX_ShapeMaterial::GetAdhesiveForce() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Surface.AdhesiveForce, GetAdhesiveForce);
}

float UAGX_ShapeMaterial::GetAdhesiveOverlap() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Surface.AdhesiveOverlap, GetAdhesiveOverlap);
}

// Bulk properties.
void UAGX_ShapeMaterial::SetDensity(float InDensity)
{
	AGX_ASSET_SETTER_F2D_IMPL(Bulk.Density, InDensity, SetDensity);
}

float UAGX_ShapeMaterial::GetDensity() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.Density, GetDensity);
}

void UAGX_ShapeMaterial::SetYoungsModulus(float InYoungsModulus)
{
	AGX_ASSET_SETTER_F2D_IMPL(Bulk.YoungsModulus, InYoungsModulus, SetYoungsModulus);
}

float UAGX_ShapeMaterial::GetYoungsModulus() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.YoungsModulus, GetYoungsModulus);
}

void UAGX_ShapeMaterial::SetBulkViscosity(float InBulkViscosity)
{
	AGX_ASSET_SETTER_F2D_IMPL(Bulk.Viscosity, InBulkViscosity, SetBulkViscosity);
}

float UAGX_ShapeMaterial::GetBulkViscosity() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.Viscosity, GetBulkViscosity);
}

void UAGX_ShapeMaterial::SetSpookDamping(float InSpookDamping)
{
	AGX_ASSET_SETTER_F2D_IMPL(Bulk.SpookDamping, InSpookDamping, SetSpookDamping);
}

float UAGX_ShapeMaterial::GetSpookDamping() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.SpookDamping, GetSpookDamping);
}

void UAGX_ShapeMaterial::SetMinMaxElasticRestLength(float InMin, float InMax)
{
	if (IsInstanceAGX())
	{
		Bulk.MinElasticRestLength = static_cast<double>(InMin);
		Bulk.MaxElasticRestLength = static_cast<double>(InMax);
		if (HasNative())
		{
			NativeBarrier->SetMinMaxElasticRestLength(
				Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);
		}
	}
	else // IsAssetAGX
	{
		if (Instance != nullptr)
		{
			Instance->SetMinMaxElasticRestLength(InMin, InMax);
			return;
		}
		Bulk.MinElasticRestLength = static_cast<double>(InMin);
		Bulk.MaxElasticRestLength = static_cast<double>(InMax);
	}
}

float UAGX_ShapeMaterial::GetMinElasticRestLength() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.MinElasticRestLength, GetMinElasticRestLength);
}

float UAGX_ShapeMaterial::GetMaxElasticRestLength() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Bulk.MaxElasticRestLength, GetMaxElasticRestLength);
}

// Wire properties.
float UAGX_ShapeMaterial::GetYoungsModulusStretch() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Wire.YoungsModulusStretch, GetYoungsModulusStretch);
}

void UAGX_ShapeMaterial::SetYoungsModulusStretch(float InYoungsModulus)
{
	AGX_ASSET_SETTER_F2D_IMPL(Wire.YoungsModulusStretch, InYoungsModulus, SetYoungsModulusStretch);
}

float UAGX_ShapeMaterial::GetYoungsModulusBend() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Wire.YoungsModulusBend, GetYoungsModulusBend);
}

void UAGX_ShapeMaterial::SetYoungsModulusBend(float InYoungsModulus)
{
	AGX_ASSET_SETTER_F2D_IMPL(Wire.YoungsModulusBend, InYoungsModulus, SetYoungsModulusBend);
}

float UAGX_ShapeMaterial::GetSpookDampingStretch() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Wire.SpookDampingStretch, GetSpookDampingStretch);
}

void UAGX_ShapeMaterial::SetSpookDampingStretch(float InSpookDamping)
{
	AGX_ASSET_SETTER_F2D_IMPL(Wire.SpookDampingStretch, InSpookDamping, SetSpookDampingStretch);
}

float UAGX_ShapeMaterial::GetSpookDampingBend() const
{
	AGX_ASSET_GETTER_D2F_IMPL(Wire.SpookDampingBend, GetSpookDampingBend);
}

void UAGX_ShapeMaterial::SetSpookDampingBend(float InSpookDamping)
{
	AGX_ASSET_SETTER_F2D_IMPL(Wire.SpookDampingBend, InSpookDamping, SetSpookDampingBend);
}

void UAGX_ShapeMaterial::CopyFrom(const FShapeMaterialBarrier* Source)
{
	if (Source)
	{
		// Copy shape material bulk properties.
		Bulk = FAGX_ShapeMaterialBulkProperties();
		Bulk.Density = Source->GetDensity();
		Bulk.YoungsModulus = Source->GetYoungsModulus();
		Bulk.Viscosity = Source->GetBulkViscosity();
		Bulk.SpookDamping = Source->GetSpookDamping();
		Bulk.MinElasticRestLength = Source->GetMinElasticRestLength();
		Bulk.MaxElasticRestLength = Source->GetMaxElasticRestLength();

		// Copy shape material surface properties.
		Surface = FAGX_ShapeMaterialSurfaceProperties();
		Surface.bFrictionEnabled = Source->GetFrictionEnabled();
		Surface.Roughness = Source->GetRoughness();
		Surface.Viscosity = Source->GetSurfaceViscosity();
		Surface.AdhesiveForce = Source->GetAdhesiveForce();
		Surface.AdhesiveOverlap = Source->GetAdhesiveOverlap();
	}
}

UAGX_MaterialBase* UAGX_ShapeMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstanceAGX())
	{
		return this;
	}

	UAGX_ShapeMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeMaterial::CreateInstanceFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_ShapeMaterial* UAGX_ShapeMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeMaterial* Source)
{
	check(Source);
	check(Source->IsAssetAGX());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_ShapeMaterial* NewInstance = NewObject<UAGX_ShapeMaterial>(
		Outer, UAGX_ShapeMaterial::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyShapeMaterialProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

FShapeMaterialBarrier* UAGX_ShapeMaterial::GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (IsAssetAGX())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateShapeMaterialNative was called on UAGX_ShapeMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
	}

	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

void UAGX_ShapeMaterial::CommitToAsset()
{
	if (IsInstanceAGX())
	{
		Asset->CopyFrom(this->GetNative());
	}
	else if (Instance != nullptr) // IsAssetAGX
	{
		Instance->CommitToAsset();
	}
}

void UAGX_ShapeMaterial::CreateNative(UWorld* PlayingWorld)
{
	if (IsAssetAGX())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on a UAGX_ShapeMaterial who's instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."));
			return;
		}
		return Instance->CreateNative(PlayingWorld);
	}

	NativeBarrier.Reset(new FShapeMaterialBarrier());
	NativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape Material '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName());
		return;
	}

	Simulation->Add(*this);
}

FShapeMaterialBarrier* UAGX_ShapeMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(IsAssetAGX());
		return Instance->GetNative();
	}

	return NativeBarrier.Get();
}

bool UAGX_ShapeMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(IsAssetAGX());
		return Instance->HasNative();
	}

	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_ShapeMaterial::UpdateNativeProperties()
{
	if (HasNative())
	{
		AGX_CHECK(IsInstanceAGX());
		NativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		// Bulk properties.
		NativeBarrier->SetDensity(Bulk.Density);
		NativeBarrier->SetYoungsModulus(Bulk.YoungsModulus);
		NativeBarrier->SetBulkViscosity(Bulk.Viscosity);
		NativeBarrier->SetSpookDamping(Bulk.SpookDamping);
		NativeBarrier->SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		// Surface properties.
		NativeBarrier->SetFrictionEnabled(Surface.bFrictionEnabled);
		NativeBarrier->SetRoughness(Surface.Roughness);
		NativeBarrier->SetSurfaceViscosity(Surface.Viscosity);
		NativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);

		// Wire properties.
		NativeBarrier->SetYoungsModulusStretch(Wire.YoungsModulusStretch);
		NativeBarrier->SetYoungsModulusBend(Wire.YoungsModulusBend);
		NativeBarrier->SetSpookDampingStretch(Wire.SpookDampingStretch);
		NativeBarrier->SetSpookDampingBend(Wire.SpookDampingBend);
	}
}

bool UAGX_ShapeMaterial::IsAssetAGX() const
{
	return !IsInstanceAGX();
}

bool UAGX_ShapeMaterial::IsInstanceAGX() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	return Asset != nullptr;
}

void UAGX_ShapeMaterial::LogBadUsage(const FString& FunctionName) const
{
	if (IsAssetAGX())
	{
		AGX_ShapeMaterial_helpers::LogBadUsage(FunctionName, "asset", "instance");
	}
	else
	{
		AGX_ShapeMaterial_helpers::LogBadUsage(FunctionName, "instance", "asset");
	}
}