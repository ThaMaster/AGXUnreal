#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_Environment
{
public:
	~FAGX_Environment();

	static FAGX_Environment& GetInstance();

	bool IsAgxDynamicsLicenseValid(FString* OutStatus = nullptr);

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

	static FString GetAgxDynamicsResourcesPath();

	FAGX_Environment(const FAGX_Environment&) = delete;
	FAGX_Environment operator=(const FAGX_Environment&) = delete;

private:
	FAGX_Environment();

	void Init();
	void SetupAGXDynamicsEnvironment();
	void LoadDynamicLibraries();

	TArray<void*> DynamicLibraryHandles;
};
