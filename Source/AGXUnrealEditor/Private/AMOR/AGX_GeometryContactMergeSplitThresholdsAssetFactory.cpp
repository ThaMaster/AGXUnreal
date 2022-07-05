// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_GeometryContactMergeSplitThresholdsAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_GeometryContactMergeSplitThresholdsAsset.h"


UAGX_GeometryContactMergeSplitThresholdsAssetFactory::
	UAGX_GeometryContactMergeSplitThresholdsAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_GeometryContactMergeSplitThresholdsAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_GeometryContactMergeSplitThresholdsAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_GeometryContactMergeSplitThresholdsAsset::StaticClass()));
	return NewObject<UAGX_GeometryContactMergeSplitThresholdsAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
