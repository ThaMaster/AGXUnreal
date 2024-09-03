// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSurfaceMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSurfaceMaterial.h"

UAGX_LidarSurfaceMaterialAssetFactory::UAGX_LidarSurfaceMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LidarSurfaceMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LidarSurfaceMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LidarSurfaceMaterial::StaticClass()));
	return NewObject<UAGX_LidarSurfaceMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
