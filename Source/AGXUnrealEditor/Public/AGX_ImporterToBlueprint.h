// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;
class FString;
struct FAGX_ImportSettings;

namespace AGX_ImporterToBlueprint
{
	/**
	 * Read simulation objects from a .agx archive or .urdf file and create an Actor
	 * Blueprint populated with corresponding AGXUnreal objects.
	 *
	 * @param ImportSettings - Struct containing all information needed to perform the import.
	 * @return An Actor Blueprint containing the read objects.
	 */
	AGXUNREALEDITOR_API UBlueprint* Import(const FAGX_ImportSettings& ImportSettings);
}
