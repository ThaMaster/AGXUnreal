// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"
#include "AGX_VectorComponent.h"
#include "AGX_CuttingEdgeComponent.generated.h"

/**
 * Specifies the lowest edge of the "active zone" of an AGX Terrain Shovel.
 *
 * Remember to also add the rigid body actor to the list of shovels on the
 * AGX Terrain's details panel!
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CuttingEdgeComponent : public UAGX_VectorComponent
{
	GENERATED_BODY()
public:
	UAGX_CuttingEdgeComponent();
};
