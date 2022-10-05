// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackInternalMergePropertiesAsset.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergePropertiesInstance.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_TrackInternalMergePropertiesInstance* UAGX_TrackInternalMergePropertiesAsset::GetInstance()
{
	return Instance.Get();
}

UAGX_TrackInternalMergePropertiesInstance*
UAGX_TrackInternalMergePropertiesAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_TrackInternalMergePropertiesInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_TrackInternalMergePropertiesInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_TrackInternalMergePropertiesAsset* UAGX_TrackInternalMergePropertiesAsset::GetAsset()
{
	return this;
}
