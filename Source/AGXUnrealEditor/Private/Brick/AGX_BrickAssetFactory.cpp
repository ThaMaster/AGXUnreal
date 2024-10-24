// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Brick/AGX_BrickAsset.h"

UAGX_BrickAssetFactory::UAGX_BrickAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_BrickAsset::StaticClass();
	bEditAfterNew = true; // Todo: what does this do?
	bCreateNew = false; // Todo: what does this do?
	bEditorImport = true;

	Formats.Add(TEXT("brick;Brick file"));
}

bool UAGX_BrickAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == UAGX_BrickAsset::StaticClass();
}

UObject* UAGX_BrickAssetFactory::FactoryCreateBinary(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	return nullptr; // Todo
}

bool UAGX_BrickAssetFactory::FactoryCanImport(const FString& Filename)
{
	return FPaths::GetExtension(Filename).Equals("brick");
}

