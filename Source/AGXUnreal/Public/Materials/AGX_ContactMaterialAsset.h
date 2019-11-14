// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "AGX_ContactMaterialAsset.generated.h"

class UAGX_ContactMaterialInstance;

/**
 * Represents Contact Material properties as an in-Editor asset that can be serialized to disk.
 * 
 * Has no connection with the actual native AGX Contact Material. Instead, its sibling class
 * UAGX_ContactMaterialInstance handles all interaction with the actual native AGX Contact Material. Therefore,
 * all in-game objects with Uproperty Contact Material pointers need to swap their pointers to in-game
 * UAGX_ContactMaterialInstances using the static function UAGX_ContactMaterialBase::GetOrCreateInstance.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ContactMaterialAsset : public UAGX_ContactMaterialBase
{
	GENERATED_BODY()

public:

	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
		
private:

	virtual UAGX_ContactMaterialAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_ContactMaterialInstance> Instance;
};
