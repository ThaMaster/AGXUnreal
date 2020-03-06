#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_EnvironmentUtilities
{
public:

	static FString GetPluginPath();

	static void AppendStringToEnvironmentVar(
		const FString& EnvironmentVarName, const FString& Value);

	static void RemoveStringFromEnvironmentVar(
		const FString& EnvironmentVarName, const FString& Value);
};
