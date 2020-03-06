#include "Utilities/AGX_EnvironmentUtilities.h"

#include "AGX_LogCategory.h"

#include "Windows/WindowsPlatformMisc.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAGX_EnvironmentUtilities"

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

	FString EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(*EnvironmentVarName);

	// Only append Value if it is not already present.
	if (EnvVarVal.Find(Value) != -1)
	{
		return;
	}

	if (!EnvVarVal.IsEmpty() && !EnvVarVal.EndsWith(";"))
	{
		EnvVarVal.Append(";");
	}

	EnvVarVal.Append(*Value);

	FWindowsPlatformMisc::SetEnvironmentVar(*EnvironmentVarName, *EnvVarVal);
}

void FAGX_EnvironmentUtilities::RemoveStringFromEnvironmentVar(
	const FString& EnvironmentVarName, FString Value)
{
	if (Value.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_EnvironmentUtilities::RemoveStringFromEnvironmentVar parameter Value was "
				 "empty."));
		return;
	}

	FString EnvVarVal = FWindowsPlatformMisc::GetEnvironmentVariable(*EnvironmentVarName);

	int32 Index = EnvVarVal.Find(Value);
	if (Index == -1)
	{
		// Value not present, do nothing.
		return;
	}

	// Include preceding semicolon if it exists.
	if (Index > 0 && EnvVarVal[Index-1] == ';')
	{
		Value.InsertAt(0, ';');
		--Index;
	}

	EnvVarVal.RemoveAt(Index, Value.Len());
	FWindowsPlatformMisc::SetEnvironmentVar(*EnvironmentVarName, *EnvVarVal);
}

#undef LOCTEXT_NAMESPACE
