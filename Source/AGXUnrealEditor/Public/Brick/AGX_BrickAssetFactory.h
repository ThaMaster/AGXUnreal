// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factories/ImportSettings.h"

#include "AGX_BrickAssetFactory.generated.h"

UCLASS()
class AGXUNREALEDITOR_API UAGX_BrickAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_BrickAssetFactory(const class FObjectInitializer& OBJ);

	//~ Begin UFactory Interface
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UObject* FactoryCreateBinary(
		UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
		FFeedbackContext* Warn) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	//~ End UFactory Interface


protected:
};
