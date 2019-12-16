#pragma once

#include "CoreMinimal.h"
#include "AGX_VectorComponent.h"
#include "AGX_CuttingDirectionComponent.generated.h"

/**
 * Specifies the cutting direction of the AGX Terrain shovel.
 *
 * The direction where the penetration resistance will be active, which is
 * usually parallel to th elowest shovel plate that is used to initially
 * penetrate the soil.
 *
 * Remember to also add the Actor to the list of shovels in the AGX Terrain's
 * details panel.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CuttingDirectionComponent : public UAGX_VectorComponent
{
	GENERATED_BODY()
public:
	UAGX_CuttingDirectionComponent();
};
