// Author: VMC Motion Technologies Co., Ltd.

#include "Vehicle/AGX_TrackPropertiesAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackPropertiesAsset.h"

UAGX_TrackPropertiesAssetFactory::UAGX_TrackPropertiesAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TrackPropertiesAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TrackPropertiesAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TrackPropertiesAsset::StaticClass()));
	return NewObject<UAGX_TrackPropertiesAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
