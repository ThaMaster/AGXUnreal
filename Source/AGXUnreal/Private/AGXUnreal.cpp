// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AGXUnreal.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealModule::StartupModule()"));

	UE_LOG(LogTemp, Log, TEXT("AGX_CALL: agx::init()"));
}

void FAGXUnrealModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealModule::ShutdownModule()"));

	UE_LOG(LogTemp, Log, TEXT("AGX_CALL: agx::shutdown()"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)