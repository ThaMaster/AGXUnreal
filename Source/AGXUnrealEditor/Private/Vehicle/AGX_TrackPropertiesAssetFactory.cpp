// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackPropertiesAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackProperties.h"

UAGX_TrackPropertiesAssetFactory::UAGX_TrackPropertiesAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TrackProperties::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TrackPropertiesAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TrackProperties::StaticClass()));
	return NewObject<UAGX_TrackProperties>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
