// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;
struct FAGX_ImporterSettings;

namespace AGX_ImporterToEditor
{
	/**
	 * Todo: Add comment.
	 */
	AGXUNREALEDITOR_API UBlueprint* Import(const FAGX_ImporterSettings& Settings);

	/**
	 * Todo: Add comment.
	 */
	AGXUNREALEDITOR_API bool Reimport(
		UBlueprint& BaseBP, const FAGX_ImporterSettings& Settings,
		UBlueprint* OpenBlueprint = nullptr);
}
