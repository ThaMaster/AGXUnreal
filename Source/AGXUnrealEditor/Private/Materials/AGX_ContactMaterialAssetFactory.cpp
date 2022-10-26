// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ContactMaterial.h"

UAGX_ContactMaterialAssetFactory::UAGX_ContactMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ContactMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ContactMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ContactMaterial::StaticClass()));
	return NewObject<UAGX_ContactMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
