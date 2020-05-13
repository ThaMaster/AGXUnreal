// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_ShapeMaterialBulkProperties.h"
#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"
#include "Materials/ShapeMaterialBarrier.h"

#include "AGX_MaterialBase.generated.h"

class FShapeMaterialBarrier;

/**
 * Defines physical properties of AGX shapes, AGX terrains etc.
 *
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", abstract, AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_MaterialBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_ShapeMaterialBulkProperties Bulk;

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_ShapeMaterialSurfaceProperties Surface;

public:
	virtual ~UAGX_MaterialBase();

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_ShapeMaterialAsset or
	 * UAGX_TerrainMaterialAsset, returns a UAGX_ShapeMaterialInstance or
	 * UAGX_TerrainMaterialInstance representing the material asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_ShapeMaterialInstance or a
	 * UAGX_TerrainMaterialInstance, it returns itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateInstance, return nullptr;);

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateShapeMaterialNative, return nullptr;);

	void CopyShapeMaterialProperties(const FShapeMaterialBarrier* Source);

protected:
	void CopyShapeMaterialProperties(const UAGX_MaterialBase* Source);
};
