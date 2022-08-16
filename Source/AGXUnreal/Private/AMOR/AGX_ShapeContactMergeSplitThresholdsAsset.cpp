// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsInstance.h"

UAGX_ShapeContactMergeSplitThresholdsBase*
UAGX_ShapeContactMergeSplitThresholdsAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ShapeContactMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_ShapeContactMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}