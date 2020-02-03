// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ShapeMaterialBase.h"
#include "Materials/MaterialBarrier.h" /// \todo Shouldn't be necessary here since we have a destructor in cpp file!
#include "AGX_ShapeMaterialInstance.generated.h"

class FMaterialBarrier;
class UAGX_ShapeMaterialAsset;

/**
 * Represents a native AGX material in-game. Should never exist when not playing.
 *
 * Should only ever be created using the static function CreateFromAsset, copying data from its
 * sibling class UAGX_ShapeMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ShapeMaterialInstance : public UAGX_ShapeMaterialBase
{
	GENERATED_BODY()

public:
	static UAGX_ShapeMaterialInstance* CreateFromAsset(UWorld* PlayingWorld, UAGX_MaterialBase* Source);

public:
	virtual ~UAGX_ShapeMaterialInstance();

	FMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	FMaterialBarrier* GetNative();

	bool HasNative() const;

	void UpdateNativeProperties();

	friend class UAGX_TerrainMaterialInstance;

private:
	virtual UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(UWorld* PlayingWorld) override;

	// Creates the native AGX material and adds it to the simulation.
	void CreateNative(UWorld* PlayingWorld);

	TUniquePtr<FMaterialBarrier> NativeBarrier;
};
