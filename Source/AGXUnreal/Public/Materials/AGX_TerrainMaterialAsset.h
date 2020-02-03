#pragma once

#include "CoreMinimal.h"

#include "Materials/AGX_TerrainMaterialBase.h"
#include "AGX_TerrainMaterialAsset.generated.h"

class UAGX_ShapeMaterialInstance;

/**
 * Defines the material for a terrain. Affects both surface and bulk properties.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_TerrainMaterialAsset : public UAGX_TerrainMaterialBase
{
public:
	/// \note Class comment above is used as tooltip in Content Browser etc, so trying to keep it
	/// simple and user-centered, while providing a more programmer-aimed comment below.

	// This class represents terrain material properties as an in-Editor asset that can be
	// serialized to disk.
	//
	// It has no connection with the actual native AGX terrain material. Instead, its sibling class
	// UAGX_TerrainMaterialInstance handles all interaction with the actual native AGX terrain
	// materials.Therefore, all in - game objects with Uproperty terrain material pointers need to
	// swap their pointers to in - game UAGX_TerrainMaterialInstances using the static function
	// UAGX_TerrainMaterialInstance::GetOrCreateInstance.

	GENERATED_BODY()

	virtual UAGX_TerrainMaterialInstance* GetOrCreateTerrainMaterialInstance(
		UWorld* PlayingWorld) override;

	virtual UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(
		UWorld* PlayingWorld) override;

private:
	TWeakObjectPtr<UAGX_TerrainMaterialInstance> TerrainMaterialInstance;
	TWeakObjectPtr<UAGX_ShapeMaterialInstance> MaterialInstance;
};
