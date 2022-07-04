// Copyright 2022, Algoryx Simulation AB.


#include "Vehicle/AGX_TrackPropertiesAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Vehicle/AGX_TrackPropertiesInstance.h"

// Unreal Engine includes.
#include "Engine/World.h"


UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesAsset::GetInstance()
{
	return Instance.Get();
}

UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_TrackPropertiesInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TrackPropertiesInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};

UAGX_TrackPropertiesAsset* UAGX_TrackPropertiesAsset::GetAsset()
{
	return this;
}
