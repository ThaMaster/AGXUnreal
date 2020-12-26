#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_EnvironmentUtilities
{
public:
	static FString GetPluginPath();

	static FString GetPluginBinariesPath();

	static FString GetProjectBinariesPath();

	static void AddEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static void RemoveEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static TArray<FString> GetEnvironmentVariableEntries(const FString& EnvVarName);

	static void SetEnvironmentVariableEntries(
		const FString& EnvVarName, const TArray<FString>& Entries);

	static bool IsSetupEnvRun();

	static FString GetAgxDynamicsResourcesPath();

	static bool IsAgxDynamicsLicenseValid(FString* OutStatus = nullptr);
};
