// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_MaterialBase.h"
#include "AGX_MaterialAsset.generated.h"

class UAGX_MaterialInstance;

/**
 * Represents material properties as an in-Editor asset that can be serialized to disk.
 * 
 * Has no connection with the actual native AGX material. Instead, its sibling class UAGX_MaterialInstance
 * handles all interaction with the actual native AGX material. Therefore, all in-game objects with 
 * Uproperty material pointers need to swap their pointers to in-game UAGX_MaterialInstances using the
 * static function UAGX_MaterialBase::GetOrCreateInstance.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_MaterialAsset : public UAGX_MaterialBase
{
	GENERATED_BODY()

public:

	virtual UAGX_MaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
		
private:

	virtual UAGX_MaterialAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_MaterialInstance> Instance;
};
