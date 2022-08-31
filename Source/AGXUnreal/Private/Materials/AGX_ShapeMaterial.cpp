// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterial.h"

// AGX Dynamics for Unreal includes.
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
		UE_LOG(LogAGX, Error, TEXT("Function '%s' was called on an %s of class UAGX_ShapeMaterial "
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
		[](ThisClass* This) { This->SetFrictionEnabled(This->Surface.bFrictionEnabled); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Roughness),
		[](ThisClass* This) {
			if (This->IsInstance())
				This->Asset->Surface.Roughness = This->Surface.Roughness;
			This->SetRoughness(This->Surface.Roughness); });

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
		[](ThisClass* This)
		{
			if (This->IsInstance())
				This->Asset->Bulk.Density = This->Bulk.Density;
			This->SetDensity(static_cast<float>(This->Bulk.Density));
		});

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

void UAGX_ShapeMaterial::SetDensity(float InDensity)
{
	if (IsInstance())
	{
		Bulk.Density = InDensity;
		if (HasNative())
		{
			NativeBarrier->SetDensity(static_cast<double>(InDensity));
		}
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->SetDensity(InDensity);
	}
}

float UAGX_ShapeMaterial::GetDensity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetDensity();
	}

	return Bulk.Density;
}

void UAGX_ShapeMaterial::SetYoungsModulus(float InYoungsModulus)
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

float UAGX_ShapeMaterial::GetYoungsModulus() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulus();
	}

	return Bulk.YoungsModulus;
}

void UAGX_ShapeMaterial::SetBulkViscosity(float InBulkViscosity)
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

float UAGX_ShapeMaterial::GetBulkViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetBulkViscosity();
	}

	return Bulk.Viscosity;
}

void UAGX_ShapeMaterial::SetSpookDamping(float InSpookDamping)
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

float UAGX_ShapeMaterial::GetSpookDamping() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDamping();
	}

	return Bulk.SpookDamping;
}

void UAGX_ShapeMaterial::SetMinMaxElasticRestLength(float InMin, float InMax)
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

float UAGX_ShapeMaterial::GetMinElasticRestLength() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMinElasticRestLength();
	}

	return Bulk.MinElasticRestLength;
}

float UAGX_ShapeMaterial::GetMaxElasticRestLength() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxElasticRestLength();
	}

	return Bulk.MaxElasticRestLength;
}

void UAGX_ShapeMaterial::SetFrictionEnabled(bool Enabled)
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

bool UAGX_ShapeMaterial::GetFrictionEnabled() const
{
	if (Instance != nullptr)
	{
		return Instance->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_ShapeMaterial::SetRoughness(float InRoughness)
{
	if (IsInstance())
	{
		Surface.Roughness = InRoughness;
		if (HasNative())
			NativeBarrier->SetRoughness(static_cast<double>(InRoughness));
	}
	else // IsAsset
	{
		if (Instance != nullptr)
		{
			Instance->SetRoughness(InRoughness);
			return;
		}
		Bulk.Density = InRoughness;
	}
}

float UAGX_ShapeMaterial::GetRoughness() const
{
	if (Instance != nullptr)
		return Instance->GetRoughness();
	if (HasNative())
		return static_cast<float>(NativeBarrier->GetRoughness());

	return static_cast<float>(Surface.Roughness);
}

void UAGX_ShapeMaterial::SetSurfaceViscosity(float Viscosity)
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

float UAGX_ShapeMaterial::GetSurfaceViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSurfaceViscosity();
	}

	return Surface.Viscosity;
}

void UAGX_ShapeMaterial::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
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

float UAGX_ShapeMaterial::GetAdhesiveForce() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveForce();
	}

	return Surface.AdhesiveForce;
}

float UAGX_ShapeMaterial::GetAdhesiveOverlap() const
{
	if (Instance != nullptr)
	{
		return Instance->GetAdhesiveOverlap();
	}

	return Surface.AdhesiveOverlap;
}

float UAGX_ShapeMaterial::GetYoungsModulusStretch() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulusStretch();
	}
	return Wire.YoungsModulusStretch;
}

void UAGX_ShapeMaterial::SetYoungsModulusStretch(float InYoungsModulus)
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

float UAGX_ShapeMaterial::GetYoungsModulusBend() const
{
	if (Instance != nullptr)
	{
		return Instance->GetYoungsModulusBend();
	}
	return Wire.YoungsModulusBend;
}

void UAGX_ShapeMaterial::SetYoungsModulusBend(float InYoungsModulus)
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

float UAGX_ShapeMaterial::GetSpookDampingStretch() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDampingStretch();
	}
	return Wire.SpookDampingStretch;
}

void UAGX_ShapeMaterial::SetSpookDampingStretch(float InSpookDamping)
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

float UAGX_ShapeMaterial::GetSpookDampingBend() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSpookDampingBend();
	}
	return Wire.SpookDampingBend;
}

void UAGX_ShapeMaterial::SetSpookDampingBend(float InSpookDamping)
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
	if (IsInstance())
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
	check(Source->IsAsset());
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

FShapeMaterialBarrier* UAGX_ShapeMaterial::GetOrCreateShapeMaterialNative(
	UWorld* PlayingWorld)
{
	if (IsAsset())
	{
		if (Instance == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("GetOrCreateShapeMaterialNative was called on a UAGX_ShapeMaterial "
				"who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling "
				"this function."));
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
	if (IsInstance())
	{
		Asset->CopyFrom(this->GetNative());
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

void UAGX_ShapeMaterial::CreateNative(UWorld* PlayingWorld)
{
	if (IsAsset())
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
		AGX_CHECK(IsAsset());
		return Instance->GetNative();
	}

	return NativeBarrier.Get();
}

bool UAGX_ShapeMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(IsAsset());
		return Instance->HasNative();
	}

	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_ShapeMaterial::UpdateNativeProperties()
{
	if (HasNative())
	{
		AGX_CHECK(IsInstance());
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



bool UAGX_ShapeMaterial::IsAsset() const
{
	return !IsInstance();
}

bool UAGX_ShapeMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	return Asset != nullptr;
}

void UAGX_ShapeMaterial::LogBadUsage(const FString& FunctionName) const
{
	if (IsAsset())
	{
		AGX_ShapeMaterial_helpers::LogBadUsage(FunctionName, "asset", "instance");
	}
	else
	{
		AGX_ShapeMaterial_helpers::LogBadUsage(FunctionName, "instance", "asset");
	}
}