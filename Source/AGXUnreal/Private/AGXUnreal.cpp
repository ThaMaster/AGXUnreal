// Copyright 2022, Algoryx Simulation AB.

#include "AGXUnreal.h"

#include "UObject/CoreRedirects.h"
#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{
	RegisterCoreRedirects();
	FAGX_RuntimeStyle::Initialize();
	FAGX_RuntimeStyle::ReloadTextures();
}

void FAGXUnrealModule::ShutdownModule()
{
	FAGX_RuntimeStyle::Shutdown();
}

void FAGXUnrealModule::RegisterCoreRedirects()
{
	// This is used to handle name changes of UFUNCTIONs, UPROPERTYs and UObjects (backward
	// compatibility). See
	// https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/ProgrammingWithCPP/Assets/CoreRedirects/

	TArray<FCoreRedirect> Redirects;

	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterial"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialAsset"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialInstance"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialBase"), TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialAsset"), TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialInstance"),
		TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialBase"),
		TEXT("AGX_TerrainMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialAsset"),
		TEXT("AGX_TerrainMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialInstance"),
		TEXT("AGX_TerrainMaterial"));

	FCoreRedirects::AddRedirectList(Redirects, TEXT("AGXUnreal"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
