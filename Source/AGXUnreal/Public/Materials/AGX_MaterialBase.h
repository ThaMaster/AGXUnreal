// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_MaterialBulkProperties.h"
#include "Materials/AGX_MaterialSurfaceProperties.h"
#include "AGX_MaterialBase.generated.h"

class UAGX_MaterialAsset;
class UAGX_MaterialInstance;

/**
 * Defines physical properties of AGX shapes etc.
 *
 * Materials are created by the user in-Editor by creating a UAGX_MaterialAsset. In-Editor they are
 * treated as assets and can be referenced by either Shapes or a Contact Materials.
 *
 * When game begins playing, one UAGX_MaterialInstance will be created for each UAGX_MaterialAsset
 * that is referenced by an in-game Shape or Contact Material. The UAGX_MaterialInstance will create
 * the actual native AGX material and add it to the simulation. The in-game Shape or Contact
 * Material that referenced the UAGX_MaterialAsset will swap its reference to the in-game created
 * UAGX_MaterialInstance instead. This means that ultimately only UAGX_MaterialInstances will be
 * referenced in-game. When play stops the in-Editor state will be returned.
 *
 * Note that this also means that UAGX_MaterialAsset that are not referenced by anything will be
 * inactive.
 *
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", abstract, AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_MaterialBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_MaterialBulkProperties Bulk;

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_MaterialSurfaceProperties Surface;

public:
	/**
	 * Invokes the member function GetOrCreateInstance() on material pointed to by Property, assigns
	 * the return value to Property, and then returns it. Returns null and does nothing if
	 * PlayingWorld is not an in-game world.
	 */
	static UAGX_MaterialInstance* GetOrCreateInstance(
		UWorld* PlayingWorld, UAGX_MaterialBase*& Property);

public:
	virtual ~UAGX_MaterialBase();

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_MaterialAsset, returns a
	 * UAGX_MaterialInstance representing the material asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_MaterialInstance it returns itself. Returns null if
	 * not in-game (invalid call).
	 */
	virtual UAGX_MaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateInstance, return nullptr;);

	/**
	 * If this material is a UAGX_MaterialInstance, returns the UAGX_MaterialAsset it was created
	 * from (if it still exists). Else returns null.
	 */
	virtual UAGX_MaterialAsset* GetAsset()
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateInstance, return nullptr;);

	void CopyProperties(const UAGX_MaterialBase* Source);
};
