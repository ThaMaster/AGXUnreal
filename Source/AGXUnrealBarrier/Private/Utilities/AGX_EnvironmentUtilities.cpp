#include "Utilities/AGX_EnvironmentUtilities.h"

#include "AGX_LogCategory.h"

#include "Windows/WindowsPlatformMisc.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAGX_EnvironmentUtilities"

namespace
{
	// Returns the value itself if it is present including preceding semicolon. Returns empty string
	// if the value is not present.
	FString FindValueWithSemicolonInEnvVar(const FString& EnvVarVal, const FString& Val)
	{
		TArray<FString> EnvVarArray;
		EnvVarVal.ParseIntoArray(EnvVarArray, TEXT(";"));

		for (int i = 0; i < EnvVarArray.Num(); i++)
		{
			if (EnvVarArray[i].Equals(Val, ESearchCase::IgnoreCase))
			{
				return i == 0 ? Val : FString(";") + Val;
			}
		}

		return FString();
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

void FAGX_EnvironmentUtilities::AppendStringToEnvironmentVar(
	const FString& EnvironmentVarName, const FString& Value)
{
	if (Value.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_EnvironmentUtilities::AppendStringToEnvironmentVar parameter Value was "
				 "empty."));
		return;
	}

	FString EnvVarVal;
#if defined(_WIN64) // \todo Add calls for other platforms
	EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(*EnvironmentVarName);
#endif

	// Only append Value if it is not already present.
	if (!FindValueWithSemicolonInEnvVar(EnvVarVal, Value).IsEmpty())
	{
		return;
	}

	if (!EnvVarVal.IsEmpty() && !EnvVarVal.EndsWith(";"))
	{
		EnvVarVal.Append(";");
	}

	EnvVarVal.Append(*Value);

#if defined(_WIN64) // \todo Add calls for other platforms
	FWindowsPlatformMisc::SetEnvironmentVar(*EnvironmentVarName, *EnvVarVal);
#endif
}

void FAGX_EnvironmentUtilities::RemoveStringFromEnvironmentVar(
	const FString& EnvironmentVarName, const FString& Value)
{
	if (Value.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_EnvironmentUtilities::RemoveStringFromEnvironmentVar parameter Value was "
				 "empty."));
		return;
	}

	FString EnvVarVal;
#if defined(_WIN64) // \todo Add calls for other platforms
	EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(*EnvironmentVarName);
#endif

	FString FoundValue = FindValueWithSemicolonInEnvVar(EnvVarVal, Value);
	if (FoundValue.IsEmpty())
		return;

	EnvVarVal.RemoveFromEnd(FoundValue);

#if defined(_WIN64) // \todo Add calls for other platforms
	FWindowsPlatformMisc::SetEnvironmentVar(*EnvironmentVarName, *EnvVarVal);
#endif
}

#undef LOCTEXT_NAMESPACE
