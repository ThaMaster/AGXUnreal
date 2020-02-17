#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_TerrainMaterialBase.h"
#include "Materials/TerrainMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"

#include "AGX_TerrainMaterialInstance.generated.h"

class UAGX_TerrainMaterialAsset;

/**
 * Represents a AGX terrain material in-game. Should never exist when not playing.
 *
 * Should only ever be created using the static function CreateFromAsset, copying data from its
 * sibling class UAGX_TerrainMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_TerrainMaterialInstance : public UAGX_TerrainMaterialBase
{
	GENERATED_BODY()

public:
	virtual ~UAGX_TerrainMaterialInstance();

	FTerrainMaterialBarrier* GetOrCreateTerrainMaterialNative(UWorld* PlayingWorld);

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;

	virtual UAGX_MaterialBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

	static UAGX_TerrainMaterialInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source);

private:

	// Creates the native AGX terrain material
	void CreateTerrainMaterialNative(UWorld* PlayingWorld);
	void CreateShapeMaterialNative(UWorld* PlayingWorld);

	bool HasTerrainMaterialNative() const;
	bool HasShapeMaterialNative() const;

	FTerrainMaterialBarrier* GetTerrainMaterialNative();
	FShapeMaterialBarrier* GetShapeMaterialNative();

	void UpdateTerrainMaterialNativeProperties();
	void UpdateShapeMaterialNativeProperties();

private:
	TUniquePtr<FTerrainMaterialBarrier> TerrainMaterialNativeBarrier;
	TUniquePtr<FShapeMaterialBarrier> ShapeMaterialNativeBarrier;
};
