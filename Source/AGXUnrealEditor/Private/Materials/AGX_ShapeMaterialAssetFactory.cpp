// Copyright 2021, Algoryx Simulation AB.


#include "Materials/AGX_ShapeMaterialAssetFactory.h"

#include "Materials/AGX_ShapeMaterialAsset.h"

UAGX_ShapeMaterialAssetFactory::UAGX_ShapeMaterialAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ShapeMaterialAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ShapeMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShapeMaterialAsset::StaticClass()));
	return NewObject<UAGX_ShapeMaterialAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
