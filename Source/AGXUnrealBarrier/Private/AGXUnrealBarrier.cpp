#include "AGXUnrealBarrier.h"

#include "AGX_LogCategory.h"
#include "Utilities/AGX_EnvironmentUtilities.h"

#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

#if defined(_WIN64) && WITH_EDITOR
#define ADD_PLUGIN_TO_PATH 1
#else
#define ADD_PLUGIN_TO_PATH 0
#endif

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

#if ADD_PLUGIN_TO_PATH
namespace
{
	void AddPluginPathToEnvironmentVar(const FString& EnvironmentVarName)
	{
		FString PluginPath = FAGX_EnvironmentUtilities::GetPluginPath();

		if (PluginPath.IsEmpty())
		{
			return;
		}

		PluginPath.Append("/Binaries/Win64");
		FAGX_EnvironmentUtilities::AppendStringToEnvironmentVar(EnvironmentVarName, PluginPath);
	}

	void RemovePluginPathFromEnvironmentVar(const FString& EnvironmentVarName)
	{
		FString PluginPath = FAGX_EnvironmentUtilities::GetPluginPath();

		if (PluginPath.IsEmpty())
		{
			return;
		}

		PluginPath.Append("/Binaries/Win64");
		FAGX_EnvironmentUtilities::RemoveStringFromEnvironmentVar(EnvironmentVarName, PluginPath);
	}
}
#endif

void FAGXUnrealBarrierModule::StartupModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::StartupModule(). Calling agx::init."));
	agx::init();

	// Start AGX logging.
	NotifyBarrier.StartAgxNotify(ELogVerbosity::Log);

#if ADD_PLUGIN_TO_PATH
	AddPluginPathToEnvironmentVar("PATH");
#endif
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
#if ADD_PLUGIN_TO_PATH
	RemovePluginPathFromEnvironmentVar("PATH");
#endif

	// Stop AGX logging.
	NotifyBarrier.StopAgxNotify();

	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule(). Calling agx::shutdown"));
	agx::shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
