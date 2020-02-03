#include "Materials/AGX_MaterialAssetFactory.h"

#include "Materials/AGX_ShapeMaterialAsset.h"

UAGX_MaterialAssetFactory::UAGX_MaterialAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ShapeMaterialAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_MaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShapeMaterialAsset::StaticClass()));
	return NewObject<UAGX_ShapeMaterialAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
