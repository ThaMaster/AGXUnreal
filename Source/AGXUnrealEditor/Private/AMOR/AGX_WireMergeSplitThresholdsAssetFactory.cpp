// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_WireMergeSplitThresholdsAsset.h"


UAGX_WireMergeSplitThresholdsAssetFactory::
	UAGX_WireMergeSplitThresholdsAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_WireMergeSplitThresholdsAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_WireMergeSplitThresholdsAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_WireMergeSplitThresholdsAsset::StaticClass()));
	return NewObject<UAGX_WireMergeSplitThresholdsAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
