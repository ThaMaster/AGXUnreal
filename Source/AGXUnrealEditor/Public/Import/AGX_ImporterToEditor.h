// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;
struct FAGX_ImporterSettings;

class AGXUNREALEDITOR_API FAGX_ImporterToEditor
{
public:
	/**
	 * Todo: Add comment.
	 */
	UBlueprint* Import(const FAGX_ImporterSettings& Settings);

	/**
	 * Todo: Add comment.
	 */
	bool Reimport(
		UBlueprint& BaseBP, const FAGX_ImporterSettings& Settings,
		UBlueprint* OpenBlueprint = nullptr);

private:
	FString RootDirectory;
	FString ModelName;
};
