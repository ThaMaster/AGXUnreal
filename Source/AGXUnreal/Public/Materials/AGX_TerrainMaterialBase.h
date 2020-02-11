#pragma once

#include "CoreMinimal.h"

#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_TerrainBulkProperties.h"
#include "Materials/AGX_TerrainCompactionProperties.h"

#include "AGX_TerrainMaterialBase.generated.h"

/**
 * Defines the material for a terrain. Affects both surface and bulk properties.
 *
 * Terrain Materials are created by the user in-Editor by creating a UAGX_TerrainMaterialAsset.
 * In-Editor they are treated as assets and can be referenced by either Terrains or Contact
 * Materials.
 *
 * When game begins playing, one UAGX_TerrainMaterialInstance will be created for each
 * UAGX_TerrainMaterialAsset that is referenced by an in-game Terrain or Contact Material. The
 * UAGX_TerrainMaterialInstance will create the actual native AGX terrain material. The in-game
 * Terrain or Contact Material that referenced the UAGX_TerrainMaterialAsset will swap its reference
 * to the in-game created instance instead. This means that ultimately only
 * UAGX_TerrainMaterialInstances will be referenced in-game. When play stops the in-Editor state
 * will be returned.
 *
 * Note that this also means that UAGX_TerrainMaterialAsset that are not
 * referenced by anything will be inactive.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_TerrainMaterialBase : public UAGX_MaterialBase
{
public:
	/**
	 * The terrain material instance can spawn both a TerrainMaterialBarrier (affecting the bulk
	 * properties of the terrain) and a ShapeMaterialBarrier (affecting the surface properties).
	 * This translates directly to AGX's native types TerrainMaterial and Material.
	 */

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainBulkProperties TerrainBulk;

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainCompactionProperties TerrainCompaction;

protected:
	void CopyTerrainMaterialProperties(const UAGX_TerrainMaterialBase* Source);
};
