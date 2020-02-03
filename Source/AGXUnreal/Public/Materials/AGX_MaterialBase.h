// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_ShapeMaterialBulkProperties.h"
#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"
#include "AGX_MaterialBase.generated.h"

class UAGX_ShapeMaterialAsset;
class UAGX_ShapeMaterialInstance;

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
	/**
	 * Invokes the member function GetOrCreateShapeMaterialInstance() on material pointed to by
	 * Property, assigns the return value to Property, and then returns it. Returns null and does
	 * nothing if PlayingWorld is not an in-game world.
	 */
	static UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(
		UWorld* PlayingWorld, UAGX_MaterialBase* Property);

public:
	virtual ~UAGX_MaterialBase();

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_ShapeMaterialAsset, returns a
	 * UAGX_ShapeMaterialInstance representing the material asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_ShapeMaterialInstance it returns itself. Returns null
	 * if not in-game (invalid call).
	 */
	virtual UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateShapeMaterialInstance, return nullptr;);

protected:
	void CopyShapeMaterialProperties(const UAGX_MaterialBase* Source);
};
