// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AGXUnreal.h"

#include "AGX_LogCategory.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::StartupModule()"));

	// TODO: We used to do agx::init here, but that is no longer allowed since
	//       this isn't the AGXUnrealBarrier module.
}

void FAGXUnrealModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::ShutdownModule()"));

	// TODO: We used to do agx::shutdown here, but that is no longer allowed
	//       since this isn't the AGXUnrealBarrierModule.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
