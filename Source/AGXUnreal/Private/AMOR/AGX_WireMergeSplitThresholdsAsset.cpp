// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_WireMergeSplitThresholdsInstance.h"

UAGX_WireMergeSplitThresholdsBase*
UAGX_WireMergeSplitThresholdsAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_WireMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_WireMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}