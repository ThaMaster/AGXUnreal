// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AGX_MaterialAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_MaterialAsset, making it possible to create asset objects in the Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_MaterialAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_MaterialAssetFactory(const class FObjectInitializer& OBJ);

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
