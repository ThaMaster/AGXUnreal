#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_Environment
{
public:
	~FAGX_Environment();

	static FAGX_Environment& GetInstance();

	/*
	 * Returns true if the AGX Dynamics environment has a valid license. Returns false otherwise. If
	 * the AGX Dynamics environment does not have a valid license, an attempt to unlock is made
	 * searching for a license file in the AGX Dynamics resources bundled with the plugin.
	 */
	bool EnsureAgxDynamicsLicenseValid(FString* OutStatus = nullptr);

	bool EnsureEnvironmentSetup();

	static FString GetPluginPath();

	static FString GetPluginBinariesPath();

	static FString GetPluginSourcePath();

	static FString GetProjectBinariesPath();

	static FString GetPluginLicenseDirPath();

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
		int32 InGeneration, int32 InMajor, int32 InMinor, int32 InPatch);

	static FString GetAGXDynamicsResourcesPath();

	FAGX_Environment(const FAGX_Environment&) = delete;
	FAGX_Environment operator=(const FAGX_Environment&) = delete;

private:
	FAGX_Environment();

	void Init();
	void SetupAGXDynamicsEnvironment();
	void LoadDynamicLibraries();
	void TryUnlockAgxDynamicsLicense();

	TArray<void*> DynamicLibraryHandles;
};
