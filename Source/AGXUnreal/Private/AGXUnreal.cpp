// Copyright 2023, Algoryx Simulation AB.

#include "AGXUnreal.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"

// Unreal Engine includes.
#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

namespace AGXUnrealModule_helpers
{
	void PrintVersion()
	{
		const FString Version = FAGX_Environment::GetPluginVersion();
		FString Revision = FAGX_Environment::GetPluginRevision();
		if (!Revision.IsEmpty())
		{
			Revision = FString::Printf(TEXT(", revision %s"), *Revision);
		}
		UE_LOG(
			LogAGX, Log, TEXT("AGX Dynamics for Unreal (AGXUnreal) version %s%s."), *Version,
			*Revision);
	}
}

void FAGXUnrealModule::StartupModule()
{
	using namespace AGXUnrealModule_helpers;
	PrintVersion();
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
	// https://docs.unrealengine.com/5.0/en-US/core-redirects-in-unreal-engine/

	TArray<FCoreRedirect> Redirects;

	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialBase"),
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
