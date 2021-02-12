// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/ContactMaterialBarrier.h" /// \todo Shouldn't be necessary here!
#include "AGX_ContactMaterialInstance.generated.h"

class FContactMaterialBarrier;
class UAGX_ContactMaterialAsset;
class UAGX_MaterialBase;

/**
 * Represents a native AGX Contact Material in-game. Should never exist when not playing.
 *
 * Should only be created using the static function CreateFromAsset, which copyies data from its
 * sibling class UAGX_ContactMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ContactMaterialInstance : public UAGX_ContactMaterialBase
{
	GENERATED_BODY()

public:
	static UAGX_ContactMaterialInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ContactMaterialAsset* Source);

public:
	virtual ~UAGX_ContactMaterialInstance();

	virtual UAGX_ContactMaterialAsset* GetAsset() override;

	FContactMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	FContactMaterialBarrier* GetNative();

	bool HasNative() const;

	void UpdateNativeProperties();

private:
	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;

	// Creates the native AGX Contact Material and adds it to the simulation.
	void CreateNative(UWorld* PlayingWorld);

	/// \todo This member is probably not necessary.. Remove it?
	TWeakObjectPtr<UAGX_ContactMaterialAsset> SourceAsset;

	TUniquePtr<FContactMaterialBarrier> NativeBarrier;
};
