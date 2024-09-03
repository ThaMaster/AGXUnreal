// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSurfaceMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_PropertyChangedDispatcher.h"


void UAGX_LidarSurfaceMaterial::SetReflectivity(float InReflectivity)
{
	if (HasNative())
	{
		NativeBarrier.SetReflectivity(InReflectivity);
	}

	Reflectivity = InReflectivity;
}

float UAGX_LidarSurfaceMaterial::GetReflectivity() const
{
	if (HasNative())
		return NativeBarrier.GetReflectivity();

	return Reflectivity;
}

bool UAGX_LidarSurfaceMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

FRtSurfaceMaterialBarrier* UAGX_LidarSurfaceMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

const FRtSurfaceMaterialBarrier* UAGX_LidarSurfaceMaterial::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

void UAGX_LidarSurfaceMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (FRtSurfaceMaterialBarrier* Barrier = GetNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier);
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

UAGX_LidarSurfaceMaterial* UAGX_LidarSurfaceMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_LidarSurfaceMaterial& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_LidarSurfaceMaterial* NewInstance = NewObject<UAGX_LidarSurfaceMaterial>(
		GetTransientPackage(), UAGX_LidarSurfaceMaterial::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

UAGX_LidarSurfaceMaterial* UAGX_LidarSurfaceMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_LidarSurfaceMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FRtSurfaceMaterialBarrier* UAGX_LidarSurfaceMaterial::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_LidarSurfaceMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

void UAGX_LidarSurfaceMaterial::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	AGX_CHECK(IsInstance());
	NativeBarrier.SetReflectivity(Reflectivity);
}

bool UAGX_LidarSurfaceMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

void UAGX_LidarSurfaceMaterial::CopyFrom(const FRtSurfaceMaterialBarrier& Source)
{
	Reflectivity = Source.GetReflectivity();
}

void UAGX_LidarSurfaceMaterial::CopyProperties(const UAGX_LidarSurfaceMaterial& Source)
{
	Reflectivity = Source.GetReflectivity();
}

void UAGX_LidarSurfaceMaterial::CreateNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on a UAGX_LidarSurfaceMaterial who's instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."));
			return;
		}
		return Instance->CreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier.HasNative())
	{
		NativeBarrier.ReleaseNative();
	}

	NativeBarrier.AllocateNative();
	check(HasNative());

	UpdateNativeProperties();
}

#if WITH_EDITOR
void UAGX_LidarSurfaceMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_LidarSurfaceMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LidarSurfaceMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSurfaceMaterial, Reflectivity),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Reflectivity, SetReflectivity) });
}
#endif
