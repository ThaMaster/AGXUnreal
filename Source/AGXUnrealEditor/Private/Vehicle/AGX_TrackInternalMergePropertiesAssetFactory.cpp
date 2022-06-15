// Author: VMC Motion Technologies Co., Ltd.

#include "Vehicle/AGX_TrackInternalMergePropertiesAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergePropertiesAsset.h"

UAGX_TrackInternalMergePropertiesAssetFactory::UAGX_TrackInternalMergePropertiesAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TrackInternalMergePropertiesAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TrackInternalMergePropertiesAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TrackInternalMergePropertiesAsset::StaticClass()));
	return NewObject<UAGX_TrackInternalMergePropertiesAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
