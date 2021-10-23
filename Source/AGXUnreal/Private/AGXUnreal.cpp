#include "AGXUnreal.h"

#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{

	UE_LOG(LogAGX, Log, TEXT("**** FAGXUnrealModule::StartupModule() entry point."));

	FAGX_RuntimeStyle::Initialize();
	FAGX_RuntimeStyle::ReloadTextures();
}

void FAGXUnrealModule::ShutdownModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::ShutdownModule()"));

	FAGX_RuntimeStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
