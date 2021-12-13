// Copyright 2021, Algoryx Simulation AB.


#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;
class FString;

namespace AGX_ImporterToBlueprint
{
	/**
	 * Read simulation objects from the .agx archive pointed to by 'ArchivePath` and create an Actor
	 * Blueprint populated with corresponding AGXUnreal objects.
	 *
	 * @param ArchivePath - The path to the AGX Dynamics archive to read.
	 * @return An Actor Blueprint containing the read objects.
	 */
	AGXUNREALEDITOR_API UBlueprint* ImportAGXArchive(const FString& ArchivePath);

	/**
	 * Read simulation objects from the .urdf file pointed to by 'UrdfFilePath` and create an Actor
	 * Blueprint populated with corresponding AGXUnreal objects.
	 *
	 * @param UrdfFilePath - The path to the URDF file to read.
	 * @param UrdfPackagePath - The path to the package directory. Corresponds to the `package://`
	 * part of any filepath in the .urdf file, typically used for pointing at mesh files. Can be
	 * left empty if the URDF file does not have any file paths in it.
	 * @return An Actor Blueprint containing the read objects.
	 */
	AGXUNREALEDITOR_API UBlueprint* ImportURDF(
		const FString& UrdfFilePath, const FString& UrdfPackagePath = "");
}
