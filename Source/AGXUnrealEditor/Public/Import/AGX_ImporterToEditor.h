// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;
struct FAGX_ImporterSettings;

namespace AGX_ImporterToEditor
{
	/**
	 * Read simulation objects from a .agx archive or .urdf file and create an Actor
	 * Blueprint populated with corresponding AGXUnreal objects.
	 *
	 * @param ImportSettings - Struct containing all information needed to perform the import.
	 * @return An Actor Blueprint containing the read objects.
	 */
	AGXUNREALEDITOR_API UBlueprint* Import(const FAGX_ImporterSettings& Settings);
}
