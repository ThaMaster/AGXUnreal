#include "AGXUnrealBarrier.h"

#include "AGX_LogCategory.h"

#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

void FAGXUnrealBarrierModule::StartupModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::StartupModule(). Calling agx::init."));
	agx::init();

	// Start AGX logging.
	NotifyBarrier.StartAgxNotify(ELogVerbosity::Log);
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
	// Stop AGX logging.
	NotifyBarrier.StopAgxNotify();

	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule(). Calling agx::shutdown"));
	agx::shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
