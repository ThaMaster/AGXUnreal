#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_EnvironmentUtilities
{
public:

	static FString GetPluginPath();

	static void AddEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static void RemoveEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static TArray<FString> GetEnvironmentVariableEntries(const FString& EnvVarName);
};
