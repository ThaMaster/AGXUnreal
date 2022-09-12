// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholds.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

void UAGX_WireMergeSplitThresholds::SetForcePropagationDecayScale_BP(
	float InForcePropagationDecayScale)
{
	SetForcePropagationDecayScale(FAGX_Real(InForcePropagationDecayScale));
}

void UAGX_WireMergeSplitThresholds::SetForcePropagationDecayScale(
	FAGX_Real InForcePropagationDecayScale)
{
	AGX_ASSET_SETTER_IMPL(
		ForcePropagationDecayScale, InForcePropagationDecayScale, SetForcePropagationDecayScale);
}

float UAGX_WireMergeSplitThresholds::GetForcePropagationDecayScale_BP() const
{
	return static_cast<float>(GetForcePropagationDecayScale());
}

FAGX_Real UAGX_WireMergeSplitThresholds::GetForcePropagationDecayScale() const
{
	AGX_ASSET_GETTER_IMPL(ForcePropagationDecayScale, GetForcePropagationDecayScale);
}

void UAGX_WireMergeSplitThresholds::SetMergeTensionScale_BP(float InMergeTensionScale)
{
	SetMergeTensionScale(FAGX_Real(InMergeTensionScale));
}

void UAGX_WireMergeSplitThresholds::SetMergeTensionScale(FAGX_Real InMergeTensionScale)
{
	AGX_ASSET_SETTER_IMPL(MergeTensionScale, InMergeTensionScale, SetMergeTensionScale);
}

float UAGX_WireMergeSplitThresholds::GetMergeTensionScale_BP() const
{
	return static_cast<float>(GetMergeTensionScale());
}

FAGX_Real UAGX_WireMergeSplitThresholds::GetMergeTensionScale() const
{
	AGX_ASSET_GETTER_IMPL(MergeTensionScale, GetMergeTensionScale);
}

#if WITH_EDITOR
void UAGX_WireMergeSplitThresholds::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_WireMergeSplitThresholds::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_WireMergeSplitThresholds::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholds, ForcePropagationDecayScale),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				ForcePropagationDecayScale, SetForcePropagationDecayScale)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholds, MergeTensionScale),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MergeTensionScale, SetMergeTensionScale) });
}
#endif

UAGX_WireMergeSplitThresholds* UAGX_WireMergeSplitThresholds::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_WireMergeSplitThresholds* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_WireMergeSplitThresholds::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

void UAGX_WireMergeSplitThresholds::CreateNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_WireMergeSplitThresholds "
					 "'%s' who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	NativeBarrier.Reset(new FWireMergeSplitThresholdsBarrier());
	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	UpdateNativeProperties();
}

FWireMergeSplitThresholdsBarrier* UAGX_WireMergeSplitThresholds::GetOrCreateNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_WireMergeSplitThresholds '%s'"
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

	return NativeBarrier.Get();
}

bool UAGX_WireMergeSplitThresholds::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_WireMergeSplitThresholds* UAGX_WireMergeSplitThresholds::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_WireMergeSplitThresholds& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());
	AGX_CHECK(!Source.IsInstance());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_WireMergeSplitThresholds>(
		Outer, UAGX_WireMergeSplitThresholds::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

bool UAGX_WireMergeSplitThresholds::IsInstance() const
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

void UAGX_WireMergeSplitThresholds::CopyProperties(UAGX_WireMergeSplitThresholds& Source)
{
	// Todo: implement.
}

void UAGX_WireMergeSplitThresholds::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}
