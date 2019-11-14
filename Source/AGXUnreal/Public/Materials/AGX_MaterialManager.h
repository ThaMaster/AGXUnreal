// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "AGX_MaterialManager.generated.h"

class UAGX_ContactMaterialBase;

/**
 * Defines which AGX Contact Materials should be used by the owning level.
 *
 * Note that this class does not yet support in-game changes to the Contact Materials array.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_MaterialManager : public AInfo
{
	GENERATED_UCLASS_BODY()
	
public:

	/**
	 * User defined AGX Contact Materials to use in this level.
	 *
	 * It is not allowed to modify this array in-game.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Manager")
	TArray<UAGX_ContactMaterialBase*> ContactMaterials;

	virtual void BeginPlay() override;

};
