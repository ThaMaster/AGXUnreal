// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ConstraintMergeSplitThresholdsInstance.h"

UAGX_ConstraintMergeSplitThresholdsBase*
UAGX_ConstraintMergeSplitThresholdsAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ConstraintMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_ConstraintMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}