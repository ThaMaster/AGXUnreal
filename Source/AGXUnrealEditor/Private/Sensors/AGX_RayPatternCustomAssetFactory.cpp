// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_RayPatternCustomAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternCustom.h"

UAGX_RayPatternCustomAssetFactory::UAGX_RayPatternCustomAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_RayPatternCustom::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_RayPatternCustomAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_RayPatternCustom::StaticClass()));
	return NewObject<UAGX_RayPatternCustom>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
