// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_ContactMaterialAssetFactory.h"

#include "Materials/AGX_ContactMaterialAsset.h"


UAGX_ContactMaterialAssetFactory::UAGX_ContactMaterialAssetFactory(const class FObjectInitializer &OBJ)
	:
Super(OBJ)
{
	SupportedClass = UAGX_ContactMaterialAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ContactMaterialAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ContactMaterialAsset::StaticClass()));
	return NewObject<UAGX_ContactMaterialAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
