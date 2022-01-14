// Copyright 2022, Algoryx Simulation AB.


#include "Materials/AGX_TerrainMaterialAssetFactory.h"

#include "Materials/AGX_TerrainMaterialAsset.h"

UAGX_TerrainMaterialAssetFactory::UAGX_TerrainMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TerrainMaterialAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TerrainMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TerrainMaterialAsset::StaticClass()));
	return NewObject<UAGX_TerrainMaterialAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
