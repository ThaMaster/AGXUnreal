// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Brick/AGX_BrickAsset.h"

// Unreal Engine includes.
#include "EditorFramework/AssetImportData.h"


UAGX_BrickAssetFactory::UAGX_BrickAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_BrickAsset::StaticClass();
	bCreateNew = false; // Todo: what does this do?
	bEditorImport = true;

	Formats.Add(TEXT("brick;Brick file"));
}

bool UAGX_BrickAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == UAGX_BrickAsset::StaticClass();
}

UObject* UAGX_BrickAssetFactory::FactoryCreateFile(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
	const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(
		this, InClass, InParent, InName, TEXT("brick"));

	bOutOperationCanceled = false;

	// Reminder: 
	// if (IsAutomatedImport()) -> don't show any dialogues, or ask for user input! See Factory.h

	auto BrickAsset = NewObject<UAGX_BrickAsset>(InParent, InClass, InName, Flags);
	BrickAsset->AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	if (!CurrentFilename.IsEmpty())
	{
		BrickAsset->AssetImportData->Update(CurrentFilename);
	}

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, BrickAsset);

	return BrickAsset;
}

bool UAGX_BrickAssetFactory::FactoryCanImport(const FString& Filename)
{
	return FPaths::GetExtension(Filename).Equals("brick");
}

bool UAGX_BrickAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (auto BrickAsset = Cast<UAGX_BrickAsset>(Obj))
	{
		BrickAsset->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}

	return false;
}

void UAGX_BrickAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	auto BrickAsset = Cast<UAGX_BrickAsset>(Obj);
	if (BrickAsset && BrickAsset->AssetImportData && ensure(NewReimportPaths.Num() == 1))
	{
		BrickAsset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UAGX_BrickAssetFactory::Reimport(UObject* Obj)
{
	return EReimportResult::Succeeded;
}

int32 UAGX_BrickAssetFactory::GetPriority() const
{
	return ImportPriority;
}

