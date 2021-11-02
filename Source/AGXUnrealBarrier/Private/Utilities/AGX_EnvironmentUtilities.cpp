#include "Utilities/AGX_EnvironmentUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Runtime.h>
#include <agx/version.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FAGX_EnvironmentUtilities"

// Create a current-platform-specific version of the OS utilities.
/// \note Something like this should be built into Unreal. Find it.
#if defined(_WIN64)
#include "Windows/WindowsPlatformMisc.h"
struct FCurrentPlatformMisc : public FWindowsPlatformMisc
{
};
#elif defined(__linux__)
#include "Linux/LinuxPlatformMisc.h"
struct FCurrentPlatformMisc : public FLinuxPlatformMisc
{
};
#else
static_assert(false);
#endif

// May return empty FString if plugin path is not found.
FString FAGX_EnvironmentUtilities::GetPluginPath()
{
	constexpr TCHAR PLUGIN_NAME[] = TEXT("AGXUnreal");

	FString AgxPluginPath;
	if (auto Plugin = IPluginManager::Get().FindPlugin(PLUGIN_NAME))
	{
		AgxPluginPath = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_EnvironmentUtilities::GetPluginPath unable to get plugin path."));
	}

	return AgxPluginPath;
}

FString FAGX_EnvironmentUtilities::GetPluginBinariesPath()
{
	const FString PluginPath = GetPluginPath();
	const FString PluginBinPath = FPaths::Combine(PluginPath, FString("Binaries"));

	return PluginBinPath;
}

FString FAGX_EnvironmentUtilities::GetProjectBinariesPath()
{
	const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	const FString ProjectBinPath = FPaths::Combine(ProjectPath, FString("Binaries"));

	return ProjectBinPath;
}

FString FAGX_EnvironmentUtilities::GetPluginVersion()
{
	constexpr TCHAR PLUGIN_NAME[] = TEXT("AGXUnreal");

	if (auto Plugin = IPluginManager::Get().FindPlugin(PLUGIN_NAME))
	{
		return Plugin->GetDescriptor().VersionName;
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_EnvironmentUtilities::GetPluginVersion unable to get plugin version."));
		return FString();
	}
}

void FAGX_EnvironmentUtilities::AddEnvironmentVariableEntry(
	const FString& EnvVarName, const FString& Entry)
{
	if (Entry.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_EnvironmentUtilities::AddEnvironmentVariableEntry parameter Entry was "
				 "empty."));
		return;
	}

	TArray<FString> EnvVarValArray = GetEnvironmentVariableEntries(EnvVarName);

	// Only append Entry if it is not already present.
	if (EnvVarValArray.Find(Entry) != -1)
	{
		return;
	}

	EnvVarValArray.Add(Entry);
	SetEnvironmentVariableEntries(EnvVarName, EnvVarValArray);
}

void FAGX_EnvironmentUtilities::RemoveEnvironmentVariableEntry(
	const FString& EnvVarName, const FString& Entry)
{
	if (Entry.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_EnvironmentUtilities::RemoveEnvironmentVariableEntry parameter Entry was "
				 "empty."));
		return;
	}

	TArray<FString> EnvVarValArray = GetEnvironmentVariableEntries(EnvVarName);
	EnvVarValArray.Remove(Entry);
	SetEnvironmentVariableEntries(EnvVarName, EnvVarValArray);
}

TArray<FString> FAGX_EnvironmentUtilities::GetEnvironmentVariableEntries(const FString& EnvVarName)
{
	FString EnvVarVal = FCurrentPlatformMisc::GetEnvironmentVariable(*EnvVarName);
	TArray<FString> EnvVarValArray;
	EnvVarVal.ParseIntoArray(EnvVarValArray, TEXT(";"), false);
	return EnvVarValArray;
}

void FAGX_EnvironmentUtilities::SetEnvironmentVariableEntries(
	const FString& EnvVarName, const TArray<FString>& Entries)
{
	FString EnvVarVal = FString::Join(Entries, TEXT(";"));
	FCurrentPlatformMisc::SetEnvironmentVar(*EnvVarName, *EnvVarVal);
}

bool FAGX_EnvironmentUtilities::IsSetupEnvRun()
{
	const TArray<FString> AgxDepDirEntries =
		FAGX_EnvironmentUtilities::GetEnvironmentVariableEntries("AGX_DEPENDENCIES_DIR");

	const TArray<FString> AgxDirEntries =
		FAGX_EnvironmentUtilities::GetEnvironmentVariableEntries("AGX_DIR");

	return AgxDepDirEntries.Num() > 0 && AgxDirEntries.Num() > 0;
}

FString FAGX_EnvironmentUtilities::GetAGXDynamicsVersion()
{
	return FString(agxGetVersion(false));
}

void FAGX_EnvironmentUtilities::GetAGXDynamicsVersion(
	int32& OutGeneration, int32& OutMajor, int32& OutMinor, int32& OutPatch)
{
	OutGeneration = AGX_GENERATION_VERSION;
	OutMajor = AGX_MAJOR_VERSION;
	OutMinor = AGX_MINOR_VERSION;
	OutPatch = AGX_PATCH_VERSION;
}

bool FAGX_EnvironmentUtilities::IsAGXDynamicsVersionNewerOrEqualTo(
	int32 InGeneration, int32 InMajor, int32 InMinor, int32 InPatch)
{
	int32 Generation, Major, Minor, Patch;
	GetAGXDynamicsVersion(Generation, Major, Minor, Patch);

	const TArray<int32> InVer {InGeneration, InMajor, InMinor, InPatch};
	const TArray<int32> AGXVer {Generation, Major, Minor, Patch};

	for (int I = 0; I < InVer.Num(); I++)
	{
		if (InVer[I] < AGXVer[I])
		{
			return true;
		}

		if (InVer[I] > AGXVer[I])
		{
			return false;
		}
	}

	// Both versions are identical.
	return true;
}

FString FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath()
{
	if (IsSetupEnvRun())
	{
		const TArray<FString> AgxDirEntries =
			FAGX_EnvironmentUtilities::GetEnvironmentVariableEntries("AGX_DIR");
		if (AgxDirEntries.Num() <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath environment variable "
					 "AGX_DIR not set when expecting setup_env to have be called. Returning empty "
					 "string."));
			return FString("");
		}

		return AgxDirEntries[0];
	}
	else
	{
		// Get and return path to AGX Dynamics resources when packaged with the plugin.
#if WITH_EDITOR
		const FString BinariesPath = FAGX_EnvironmentUtilities::GetPluginBinariesPath();
#else
		// This is the correct binaries path when running as a built executable.
		const FString BinariesPath = FAGX_EnvironmentUtilities::GetProjectBinariesPath();
#endif
		const FString AgxResourcesPath =
			FPaths::Combine(BinariesPath, FString("ThirdParty"), FString("agx"));

		return AgxResourcesPath;
	}
}

void FAGX_EnvironmentUtilities::EnsureAgxDynamicsEnvironmentIsSetup()
{
	if (FAGX_EnvironmentUtilities::IsSetupEnvRun())
	{
		// Only setup AGX Dynamics environment paths if setup_env has not been called.
		return;
	}

	static bool IsEnvironmentSetup = false;
	if (IsEnvironmentSetup)
	{
		// Only setup AGX Dynamics environment paths once.
		return;
	}

	const FString AgxResourcesPath = FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath();
	const FString AgxBinPath = FPaths::Combine(AgxResourcesPath, FString("bin"));
	const FString AgxDataPath = FPaths::Combine(AgxResourcesPath, FString("data"));
	const FString AgxCfgPath = FPaths::Combine(AgxDataPath, FString("cfg"));
	FString AgxPluginsPath = FPaths::Combine(AgxResourcesPath, FString("plugins"));

	if (!FPaths::DirectoryExists(AgxPluginsPath))
	{
		AgxPluginsPath = FPaths::Combine(AgxBinPath, FString("plugins"));
	}

	UE_LOG(
		LogAGX, Log,
		TEXT("No installation of AGX Dynamics detected. Using AGX Dynamics resources from the "
			 "AGXUnreal plugin at: %s"),
		*AgxResourcesPath);

	// Ensure that the necessary AGX Dynamics resources are packed with the plugin.
	if (!FPaths::DirectoryExists(AgxResourcesPath) || !FPaths::DirectoryExists(AgxBinPath) ||
		!FPaths::DirectoryExists(AgxDataPath) || !FPaths::DirectoryExists(AgxCfgPath) ||
		!FPaths::DirectoryExists(AgxPluginsPath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Dynamics resources are not packaged with the AGXUnreal plugin. The "
				 "plugin will not be able to load AGX Dynamics. The resources where expected "
				 "to be at: %s"),
			*AgxResourcesPath);

		// This will likely result in a runtime error since the needed AGX Dynamics resources
		// are nowhere to be found.
		return;
	}

	// If AGX Dynamics is installed on this computer, agxIO.Environment.instance() will
	// read data from the registry and add runtime and resource paths to
	// the installed version (even if setup_env has not been called). Clear all, from registry
	// added paths since we will use the AGX Dynamics resources packed with the plugin only.
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

	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RESOURCE_PATH)
		.pushbackPath(Convert(AgxCfgPath));

	IsEnvironmentSetup = true;
}

bool FAGX_EnvironmentUtilities::IsAgxDynamicsLicenseValid(FString* OutStatus)
{
	// The AGX Dynamics environment has to be setup before calling `agx::Runtime::instance()` since
	// the first call to that function will trigger the search for a AGX Dynamics license inside AGX
	// Dynamics.
	FAGX_EnvironmentUtilities::EnsureAgxDynamicsEnvironmentIsSetup();

	bool LicenseValid = false;
	if (agx::Runtime* AgxRuntime = agx::Runtime::instance())
	{
		LicenseValid = AgxRuntime->isValid();
		if (OutStatus)
		{
			const FString Status = Convert(AgxRuntime->getStatus());
			*OutStatus = Status;
		}
	}

	return LicenseValid;
}

#undef LOCTEXT_NAMESPACE
