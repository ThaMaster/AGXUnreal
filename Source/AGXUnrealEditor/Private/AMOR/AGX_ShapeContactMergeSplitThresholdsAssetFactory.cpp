// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAsset.h"


UAGX_ShapeContactMergeSplitThresholdsAssetFactory::
	UAGX_ShapeContactMergeSplitThresholdsAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ShapeContactMergeSplitThresholdsAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ShapeContactMergeSplitThresholdsAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShapeContactMergeSplitThresholdsAsset::StaticClass()));
	return NewObject<UAGX_ShapeContactMergeSplitThresholdsAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
