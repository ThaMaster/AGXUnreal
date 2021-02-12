#include "AGXUnrealBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EnvironmentUtilities.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformProcess.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

namespace
{
	void LogAgxDynamicsLicenseStatus()
	{
		FString Status;
		if (FAGX_EnvironmentUtilities::IsAgxDynamicsLicenseValid(&Status) == false)
		{
			UE_LOG(LogAGX, Error, TEXT("AGX Dynamics license is invalid. Status: %s"), *Status);
		}
		else
		{
			UE_LOG(LogAGX, Log, TEXT("AGX Dynamics license is valid."));
		}
	}
}

void FAGXUnrealBarrierModule::StartupModule()
{
	// SetupAgxEnvironment must be called before agx::init().
	SetupAgxEnvironment();

	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealBarrierModule::StartupModule(). Calling agx::init."));
	agx::init();

	// Start AGX logging.
	NotifyBarrier.StartAgxNotify(ELogVerbosity::Log);

	LogAgxDynamicsLicenseStatus();
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
	// Stop AGX logging.
	NotifyBarrier.StopAgxNotify();

	UE_LOG(
		LogAGX, Log, TEXT("FAGXUnrealBarrierModule::ShutdownModule(). Calling agx::shutdown"));
	agx::shutdown();

	if (VdbGridLibHandle)
	{
		FPlatformProcess::FreeDllHandle(VdbGridLibHandle);
		VdbGridLibHandle = nullptr;
	}
}

void FAGXUnrealBarrierModule::SetupAgxEnvironment()
{
	// Check if an AGX environment is already set up, in that case we do not have to do anything
	// more here.
	if (FAGX_EnvironmentUtilities::IsSetupEnvRun())
	{
		const FString AgxDynamicsResoucePath =
			FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath();
		UE_LOG(
			LogAGX, Log, TEXT("AGX Dynamics installation was detected. Using resources from: %s"),
			*AgxDynamicsResoucePath);
	}
	else
	{
		// setup_env not called, use the AGX resources packaged with the plugin.
		SetupUsePluginResourcesOnly();
	}
}

// In case this process is run without an AGX Dynamics environment (setup_env has not been run),
// setup the AGX environment to use the necessary AGX Dynamics resources that is packaged with
// the plugin itself, if they exist.
void FAGXUnrealBarrierModule::SetupUsePluginResourcesOnly()
{
	check(FAGX_EnvironmentUtilities::IsSetupEnvRun() == false);
	const FString AgxResourcesPath = FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath();
	const FString AgxBinPath = FPaths::Combine(AgxResourcesPath, FString("bin"));

#if defined(_WIN64)
	const FString VdbGridLibPath =
		FPaths::Combine(AgxBinPath, FString("Win64"), FString("vdbgrid.dll"));
#endif
#if defined(__linux__)
	const FString VdbGridLibPath =
		FPaths::Combine(AgxBinPath, FString("Linux"), FString("libvdbgrid.so"));
#endif
	// vdbgrid.dll is loaded dynamically at runtime by AGX Dynamics's Terrain module. The directory
	// containing vdbgrid.dll must either be in PATH or it can be pre-loaded which is done here.
	// The result of not doing either is a runtime crash when using certain AGX Dynamics Terrain
	// features.
	VdbGridLibHandle = FPlatformProcess::GetDllHandle(*VdbGridLibPath);

	if (!VdbGridLibHandle)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to dynamically load %s but the loading failed. Some AGX Dynamics terrain "
				 "features might not be available."));
	}

	FAGX_EnvironmentUtilities::EnsureAgxDynamicsEnvironmentIsSetup();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
