#include "Utilities/AGX_EnvironmentUtilities.h"

#include "AGX_LogCategory.h"

#include "Windows/WindowsPlatformMisc.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAGX_EnvironmentUtilities"

namespace
{
	TArray<FString> GetEnvironmentVariableEntries(const FString& EnvVarName)
	{
#if defined(_WIN64)
		FString EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(*EnvVarName);
#else
		// \todo Add calls for other platforms
		static_assert(false);
#endif

		TArray<FString> EnvVarValArray;
		EnvVarVal.ParseIntoArray(EnvVarValArray, TEXT(";"), false);

		return EnvVarValArray;
	}

	void WriteEnvironmentVariable(const FString& EnvVarName, const TArray<FString>& Entries)
	{
		FString EnvVarVal = FString::Join(Entries, TEXT(";"));
#if defined(_WIN64)
		FWindowsPlatformMisc::SetEnvironmentVar(*EnvVarName, *EnvVarVal);
#else
		// \todo Add calls for other platforms
		static_assert(false);
#endif
	}
}

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
	WriteEnvironmentVariable(EnvVarName, EnvVarValArray);
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
	WriteEnvironmentVariable(EnvVarName, EnvVarValArray);
}

#undef LOCTEXT_NAMESPACE
