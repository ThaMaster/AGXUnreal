// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_RayPatternHorizontalSweepAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternHorizontalSweep.h"

UAGX_RayPatternHorizontalSweepAssetFactory::UAGX_RayPatternHorizontalSweepAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_RayPatternHorizontalSweep::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_RayPatternHorizontalSweepAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_RayPatternHorizontalSweep::StaticClass()));
	return NewObject<UAGX_RayPatternHorizontalSweep>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
