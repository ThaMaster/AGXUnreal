// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "Factories/Factory.h"
#include "Factories/ImportSettings.h"

#include "AGX_BrickAssetFactory.generated.h"

UCLASS()
class AGXUNREALEDITOR_API UAGX_BrickAssetFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UAGX_BrickAssetFactory(const class FObjectInitializer& OBJ);

	//~ Begin UFactory Interface
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UObject* FactoryCreateFile(
		UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn,
		bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	//~ End UFactory Interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(
		UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface
};
