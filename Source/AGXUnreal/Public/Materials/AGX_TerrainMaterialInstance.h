#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_TerrainMaterialBase.h"
#include "Materials/TerrainMaterialBarrier.h"

#include "AGX_TerrainMaterialInstance.generated.h"

class UAGX_TerrainMaterialAsset;
class UAGX_ShapeMaterialInstance;

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

	FTerrainMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	virtual UAGX_TerrainMaterialInstance* GetOrCreateTerrainMaterialInstance(
		UWorld* PlayingWorld) override;

	static UAGX_TerrainMaterialInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source);

private:
	// Must override pure virtual, but should never be called on a AGX_TerrainMaterialInstance
	virtual UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(
		UWorld* PlayingWorld) override;

	// Creates the native AGX terrain material
	void CreateNative(UWorld* PlayingWorld);

	bool HasNative() const;

	FTerrainMaterialBarrier* GetNative();

	void UpdateNativeProperties();

private:
	TUniquePtr<FTerrainMaterialBarrier> NativeBarrier;
};
