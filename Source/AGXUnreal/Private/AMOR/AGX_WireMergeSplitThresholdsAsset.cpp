// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AMOR/AGX_WireMergeSplitThresholdsInstance.h"

UAGX_WireMergeSplitThresholdsBase* UAGX_WireMergeSplitThresholdsAsset::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	UAGX_WireMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_WireMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_WireMergeSplitThresholdsAsset::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_WireMergeSplitThresholdsAsset::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_WireMergeSplitThresholdsAsset::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholdsBase, ForcePropagationDecayScale),
		[](ThisClass* This)
		{ This->SetForcePropagationDecayScale(This->ForcePropagationDecayScale); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholdsBase, MergeTensionScale),
		[](ThisClass* This) { This->SetMergeTensionScale(This->MergeTensionScale); });
}
#endif

void UAGX_WireMergeSplitThresholdsAsset::SetForcePropagationDecayScale(
	FAGX_Real InForcePropagationDecayScale)
{
	if (Instance != nullptr)
	{
		Instance->SetForcePropagationDecayScale(InForcePropagationDecayScale);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		ForcePropagationDecayScale = InForcePropagationDecayScale;
	}
}

FAGX_Real UAGX_WireMergeSplitThresholdsAsset::GetForcePropagationDecayScale() const
{
	if (Instance != nullptr)
	{
		return Instance->GetForcePropagationDecayScale();
	}

	return ForcePropagationDecayScale;
}

void UAGX_WireMergeSplitThresholdsAsset::SetMergeTensionScale(FAGX_Real InMergeTensionScale)
{
	if (Instance != nullptr)
	{
		Instance->SetMergeTensionScale(InMergeTensionScale);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MergeTensionScale = InMergeTensionScale;
	}
}

FAGX_Real UAGX_WireMergeSplitThresholdsAsset::GetMergeTensionScale() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMergeTensionScale();
	}

	return MergeTensionScale;
}