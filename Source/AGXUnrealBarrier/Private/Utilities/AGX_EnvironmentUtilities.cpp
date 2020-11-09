#include "Utilities/AGX_EnvironmentUtilities.h"

#include "AGX_LogCategory.h"

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

#undef LOCTEXT_NAMESPACE
