#include "AGXUnreal.h"

#include "UObject/CoreRedirects.h"
#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::StartupModule()"));

	RegisterCoreRedirects();
	FAGX_RuntimeStyle::Initialize();
	FAGX_RuntimeStyle::ReloadTextures();
}

void FAGXUnrealModule::ShutdownModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::ShutdownModule()"));

	FAGX_RuntimeStyle::Shutdown();
}

void FAGXUnrealModule::RegisterCoreRedirects()
{
	// This is used to handle name changes of UFUNCTIONs, UPROPERTYs and UObjects (backward
	// compatibility).

	// Uncomment below once the first Core Redirect is added. Remove this comment when that happens.

	/*
	TArray<FCoreRedirect> Redirects;

	Redirects.Emplace(
		ECoreRedirectFlags::Type_Property, TEXT("AGX_ExampleComponent.OldExampleProperty"),
		TEXT("NewExampleProperty"));

	FCoreRedirects::AddRedirectList(Redirects, TEXT("AGXUnreal"));
	*/
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
