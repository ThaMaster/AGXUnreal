#pragma once

#include "CoreMinimal.h"

#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_TerrainMaterialProperties.h"
#include "AGX_TerrainMaterialBase.generated.h"

class UAGX_TerrainMaterialInstance;

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
	 * The terrain material is represented internally as one terrain material (affecting the bulk
	 * properties of the terrain), and one shape material (affecting the surface properties). This
	 * translates directly to Agx's native types TerrainMaterial and Material.
	 */

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainMaterialProperties Terrain;

	/**
	 * Invokes the member function GetOrCreateTerrainMaterialInstance() on terrain material pointed
	 * to by Property, assigns the return value to Property, and then returns it. Returns null and
	 * does nothing if PlayingWorld is not an in-game world.
	 */
	static UAGX_TerrainMaterialInstance* GetOrCreateTerrainMaterialInstance(
		UWorld* PlayingWorld, UAGX_TerrainMaterialBase* Property);

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_TerrainMaterialAsset, returns
	 * a UAGX_TerrainMaterialInstance representing the material asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_TerrainMaterialInstance it returns itself. Returns null if
	 * not in-game (invalid call).
	 */
	virtual UAGX_TerrainMaterialInstance* GetOrCreateTerrainMaterialInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_TerrainMaterialBase::GetOrCreateTerrainMaterialInstance, return nullptr;);
};
