// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_MaterialAssetFactory.h"

#include "Materials/AGX_MaterialAsset.h"


UAGX_MaterialAssetFactory::UAGX_MaterialAssetFactory(const class FObjectInitializer &OBJ)
	:
Super(OBJ)
{
	SupportedClass = UAGX_MaterialAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_MaterialAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_MaterialAsset::StaticClass()));
	return NewObject<UAGX_MaterialAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
