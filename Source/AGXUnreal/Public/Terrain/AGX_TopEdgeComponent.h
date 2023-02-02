// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_VectorComponent.h"
#include "AGX_TopEdgeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TopEdgeComponent : public UAGX_VectorComponent
{
	GENERATED_BODY()
public:
	UAGX_TopEdgeComponent();
};
