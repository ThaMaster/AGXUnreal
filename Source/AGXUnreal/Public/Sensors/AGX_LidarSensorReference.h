// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_LidarSensorReference.generated.h"

class UAGX_LidarSensorComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_LidarSensorReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_LidarSensorReference();

	UAGX_LidarSensorComponent* GetLidarComponent() const;
};
