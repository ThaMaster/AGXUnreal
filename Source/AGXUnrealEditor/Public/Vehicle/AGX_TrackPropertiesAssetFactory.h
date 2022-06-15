// Author: VMC Motion Technologies Co., Ltd.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AGX_TrackPropertiesAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_TrackPropertiesAsset, making it possible to create asset objects in the
 * Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_TrackPropertiesAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_TrackPropertiesAssetFactory(const class FObjectInitializer& OBJ);

protected:
	virtual bool IsMacroFactory() const
	{
		return false;
	}

public:
	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
};
