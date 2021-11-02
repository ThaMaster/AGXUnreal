#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_EnvironmentUtilities
{
public:
	static FString GetPluginPath();

	static FString GetPluginBinariesPath();

	static FString GetProjectBinariesPath();

	static FString GetPluginVersion();

	static void AddEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static void RemoveEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static TArray<FString> GetEnvironmentVariableEntries(const FString& EnvVarName);

	static void SetEnvironmentVariableEntries(
		const FString& EnvVarName, const TArray<FString>& Entries);

	static bool IsSetupEnvRun();

	static FString GetAGXDynamicsVersion();

	static void GetAGXDynamicsVersion(
		int32& OutGeneration, int32& OutMajor, int32& OutMinor, int32& OutPatch);

	static bool IsAGXDynamicsVersionNewerOrEqualTo(
		int32 Generation, int32 Major, int32 Minor, int32 Patch);

	static FString GetAgxDynamicsResourcesPath();

	static void EnsureAgxDynamicsEnvironmentIsSetup();

	static bool IsAgxDynamicsLicenseValid(FString* OutStatus = nullptr);
};
