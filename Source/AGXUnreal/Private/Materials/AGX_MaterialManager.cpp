// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_MaterialManager.h"

#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/AGX_ContactMaterialInstance.h"

#define LOCTEXT_NAMESPACE "AAGX_MaterialManager"

AAGX_MaterialManager::AAGX_MaterialManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> ViewportIconTextureObject;
			FName Id;
			FText Name;

			FConstructorStatics()
				: ViewportIconTextureObject(TEXT("/Engine/EditorResources/S_Note"))
				, Id(TEXT("AGX_MaterialManager"))
				, Name(LOCTEXT("ViewportIcon", "AGX Material Manager"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (GetSpriteComponent())
		{
			GetSpriteComponent()->Sprite = ConstructorStatics.ViewportIconTextureObject.Get();
			GetSpriteComponent()->SpriteInfo.Category = ConstructorStatics.Id;
			GetSpriteComponent()->SpriteInfo.DisplayName = ConstructorStatics.Name;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void AAGX_MaterialManager::BeginPlay()
{
	// Convert all contact material pointers to point to initialized contact material instances.
	for (UAGX_ContactMaterialBase*& ContactMaterial : ContactMaterials)
	{
		if (!ContactMaterial)
		{
			continue;
		}

		// This will create the UAGX_ContactMaterialInstance if it did not already exist, initialize
		// its AGX native, and add it to the simulation.

		// It will also replace the passed in UAGX_ContactMaterialBase pointer with the new
		// instance, and return it.

		UAGX_ContactMaterialInstance* Instance =
			UAGX_ContactMaterialBase::GetOrCreateInstance(GetWorld(), ContactMaterial);
	}
}

#undef LOCTEXT_NAMESPACE
