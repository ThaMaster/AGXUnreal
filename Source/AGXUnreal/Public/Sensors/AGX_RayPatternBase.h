// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "PlayRecord/AGX_PlayRecordState.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_RayPatternBase.generated.h"

/**
 * This asset determines what ray pattern is used by a Lidar Sensor and holds all properties
 * related to that ray pattern.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_RayPatternBase : public UObject
{
	GENERATED_BODY()
};
