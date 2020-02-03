#pragma once

#include "CoreMinimal.h"

#include "AGX_TerrainMaterialProperties.generated.h"

/**
 * Terrain specific physical properties.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainMaterialProperties
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * Density of the terrain bulk, in kg/m^2.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double TerrainDensity;

public:
	FAGX_TerrainMaterialProperties();
};
