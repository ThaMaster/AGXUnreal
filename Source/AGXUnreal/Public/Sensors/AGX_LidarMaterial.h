// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarMaterial.generated.h"

/**
 * This asset represents a Material that determines the interaction with Lidar laser rays as they
 * hit an object with this Material assigned to it.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_LidarMaterial : public UObject
{
	GENERATED_BODY()
};
