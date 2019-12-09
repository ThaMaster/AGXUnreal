

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "AGX_VectorComponent.generated.h"

/**
 * A VectorComponent is used to represent a direction or an edge in the world
 * It is used by the terrain shovel to specify cutting edges and such. It comes
 * with editor integration for easier setup.
 *
 * This class is heavily influenced by the Unreal Engine ArrowComponent.
 */
UCLASS(ClassGroup="AGX", Category="AGX")
class AGXUNREAL_API UAGX_VectorComponent : public UPrimitiveComponent
{
	GENERATED_BODY()




};
