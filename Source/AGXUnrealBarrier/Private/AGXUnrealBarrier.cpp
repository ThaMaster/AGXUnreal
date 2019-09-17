// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AGXUnrealBarrier.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

void FAGXUnrealBarrierModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealBarrierModule::StartupModule()"));
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule()"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
