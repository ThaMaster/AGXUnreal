// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/MaterialBarrier.h" /// \todo Shouldn't be necessary here since we have a destructor in cpp file!
#include "AGX_MaterialInstance.generated.h"

class FMaterialBarrier;
class UAGX_MaterialAsset;

/**
 * Represents a native AGX material in-game. Should never exist when not playing.
 * 
 * Should only ever be created using the static function CreateFromAsset, copying data from its
 * sibling class UAGX_MaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_MaterialInstance : public UAGX_MaterialBase
{
	GENERATED_BODY()
	
public:

	static UAGX_MaterialInstance* CreateFromAsset(UWorld* PlayingWorld, UAGX_MaterialAsset* Source);

public:

	virtual ~UAGX_MaterialInstance();

	virtual UAGX_MaterialAsset* GetAsset() override;

	FMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	FMaterialBarrier* GetNative();

	bool HasNative() const;
	
	void UpdateNativeProperties();

private:

	virtual UAGX_MaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;

	// Creates the native AGX material and adds it to the simulation.
	void CreateNative(UWorld* PlayingWorld);

	/// \todo This member is probably not necessary.. Remove it?
	TWeakObjectPtr<UAGX_MaterialAsset> SourceAsset;

	TUniquePtr<FMaterialBarrier> NativeBarrier;
};
