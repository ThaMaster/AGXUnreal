#include "AGXUnrealBarrier.h"

#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

void FAGXUnrealBarrierModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealBarrierModule::StartupModule(). Calling agx::init."));
	agx::init();
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule(). Calling agx::shutdown"));
	agx::shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
