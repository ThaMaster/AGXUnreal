#include "AGXUnrealBarrier.h"

#include "AGX_LogCategory.h"

#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

#if defined(_WIN64) && WITH_EDITOR
#define ADD_PLUGIN_TO_PATH 1
#else
#define ADD_PLUGIN_TO_PATH 0
#endif

#if ADD_PLUGIN_TO_PATH
#include "Windows/WindowsPlatformMisc.h"
#include "Interfaces/IPluginManager.h"
#endif

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

#if ADD_PLUGIN_TO_PATH
namespace
{
	// May return empty FString if plugin path is not found.
	FString GetPluginPath()
	{
		constexpr TCHAR PLUGIN_NAME[] = TEXT("AGXUnreal");

		FString AgxPluginPath;
		if (auto Plugin = IPluginManager::Get().FindPlugin(PLUGIN_NAME))
		{
			AgxPluginPath = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
			AgxPluginPath.Append("/Binaries/Win64");
		}

		return AgxPluginPath;
	}

	void AddPluginPathToEnvironmentVar(const TCHAR* EnvironmentVarName)
	{
		FString EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(EnvironmentVarName);
		FString AgxPluginPath = GetPluginPath();

		if (EnvVarVal.IsEmpty() || AgxPluginPath.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX_UnrealBarrier::AddPluginPathToEnvironmentVar unable to add plugin path "
					 "to environment variable: %s"),
				EnvironmentVarName);
			return;
		}

		// Only add the plugin path if it is not already present.
		if (EnvVarVal.Find(AgxPluginPath) == -1)
		{
			EnvVarVal.Append(";").Append(*AgxPluginPath);
			FWindowsPlatformMisc::SetEnvironmentVar(EnvironmentVarName, *EnvVarVal);
		}
	}

	void RemovePluginPathFromEnvironmentVar(const TCHAR* EnvironmentVarName)
	{
		FString EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(EnvironmentVarName);
		FString AgxPluginPath = GetPluginPath();

		if (EnvVarVal.IsEmpty() || AgxPluginPath.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX_UnrealBarrier::RemovePluginPathToEnvironmentVar unable to remove plugin "
					 "path from environment variable: %s"),
				EnvironmentVarName);
			return;
		}

		// AgxPluginPath was added with a semicolon at the start.
		AgxPluginPath.InsertAt(0, ";");

		int32 Index = EnvVarVal.Find(AgxPluginPath);
		if (Index == -1)
		{
			// Path not present, do nothing.
			return;
		}

		EnvVarVal.RemoveAt(Index, AgxPluginPath.Len());
		FWindowsPlatformMisc::SetEnvironmentVar(EnvironmentVarName, *EnvVarVal);
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
	AddPluginPathToEnvironmentVar(TEXT("PATH"));
#endif
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
#if ADD_PLUGIN_TO_PATH
	RemovePluginPathFromEnvironmentVar(TEXT("PATH"));
#endif

	// Stop AGX logging.
	NotifyBarrier.StopAgxNotify();

	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule(). Calling agx::shutdown"));
	agx::shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
