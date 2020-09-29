#include "AGXUnrealBarrier.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EnvironmentUtilities.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include <agxIO/Environment.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Paths.h"

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

#if defined(_WIN64)
		PluginPath.Append("/Binaries/Win64");
#else
		// \todo Add calls for other platforms (UGameplayStatics::GetPlatformName() only returns
		// full name, e.g. 'Windows'. Perhaps there are some other ways to get specific platform).
		static_assert(false);
#endif
		FAGX_EnvironmentUtilities::AddEnvironmentVariableEntry(EnvironmentVarName, PluginPath);
	}

	void RemovePluginPathFromEnvironmentVar(const FString& EnvironmentVarName)
	{
		FString PluginPath = FAGX_EnvironmentUtilities::GetPluginPath();

		if (PluginPath.IsEmpty())
		{
			return;
		}

#if defined(_WIN64)
		PluginPath.Append("/Binaries/Win64");
#else
		// \todo Add calls for other platforms (UGameplayStatics::GetPlatformName() only returns
		// full name, e.g. 'Windows'. Perhaps there are some other ways to get specific platform).
		static_assert(false);
#endif
		FAGX_EnvironmentUtilities::RemoveEnvironmentVariableEntry(EnvironmentVarName, PluginPath);
	}
}
#endif

namespace SetupAgxEnvironment_helper
{
	// In case this process is ran without an AGX Dynamics environment (setup_env has not been run),
	// setup the AGX environment to use the necessary AGX resources that comes with the plugin
	// itself, if they exist.
	void SetupUsePluginResourcesOnly()
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("No installation of AGX Dynamics detected. Using AGX Dynamics resources from the "
				 "AGXUnreal plugin."));

		const FString BinariesPath = FAGX_EnvironmentUtilities::GetProjectBinariesPath();
		const FString AgxResourcesPath = FPaths::Combine(BinariesPath, FString("agx"));
		const FString AgxDataPath = FPaths::Combine(AgxResourcesPath, FString("data"));
		const FString AgxPluginsPath = FPaths::Combine(AgxResourcesPath, FString("plugins"));

		// Ensure that the necessary AGX Dynamics resources are packed with the plugin.
		if (!FPaths::DirectoryExists(AgxResourcesPath) || !FPaths::DirectoryExists(AgxDataPath) ||
			!FPaths::DirectoryExists(AgxPluginsPath))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("No AGX Dynamics resources found in the AGXUnreal plugin. The plugin will not "
					 "be able to load AGX."));

			// This will likely result in a runtime error since the needed AGX resources are nowhere
			// to be found.
			// @todo What to do here, simply continue and crash or can we throw some type of
			// exception?
			return;
		}

		// If AGX Dynamics is installed on this computer, agxIO.Environment.instance() will
		// read data from the registry and add runtime and resource paths to
		// the installed version (even if setup_env has not been called). Clear all, from registry
		// added paths since we will use the AGX resources packed with the plugin only.
		for (int i = 0; i < (int) agxIO::Environment::Type::NUM_TYPES; i++)
		{
			AGX_ENVIRONMENT().getFilePath((agxIO::Environment::Type) i).clear();
		}

		// Point the AGX environment to the resources packed with the plugin.
		AGX_ENVIRONMENT()
			.getFilePath(agxIO::Environment::RUNTIME_PATH)
			.pushbackPath(Convert(AgxPluginsPath));

		AGX_ENVIRONMENT()
			.getFilePath(agxIO::Environment::RESOURCE_PATH)
			.pushbackPath(Convert(AgxResourcesPath));

		AGX_ENVIRONMENT()
			.getFilePath(agxIO::Environment::RESOURCE_PATH)
			.pushbackPath(Convert(AgxDataPath));
	}

	void SetupAgxEnvironment()
	{
		const TArray<FString> AgxDirEntries =
			FAGX_EnvironmentUtilities::GetEnvironmentVariableEntries("AGX_DIR");

		// Check if an AGX environment is already set up, in that case we do not have to do anything
		// more here.
		if (AgxDirEntries.Num() > 0)
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("AGX Dynamics installation was detected. Using resources from: %s"),
				*AgxDirEntries[0]);
		}
		else
		{
			// No AGX environment found, use the AGX resources packed with the plugin.
			SetupUsePluginResourcesOnly();
		}
	}
}

void FAGXUnrealBarrierModule::StartupModule()
{
	SetupAgxEnvironment_helper::SetupAgxEnvironment();

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
