// Copyright 2021, Algoryx Simulation AB.


#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AActor;
class FString;

class AGXUNREALEDITOR_API AGX_ImporterToSingleActor
{
public:
	/**
	 * Read simulation objects from the .agx archive poitned to by 'ArchivePath' and create an Actor
	 * populated with corresponding AGXUnreal Component objects.
	 *
	 * @param ArchivePath - The path to the AGX Dynamics archive to read.
	 * @return An Actor containing Components for each object read.
	 */
	static AActor* ImportAGXArchive(const FString& ArchivePath);

	/**
	 * Read simulation objects from the .urdf file pointed to by 'UrdfFilePath` and create an Actor
	 * populated with corresponding AGXUnreal objects.
	 *
	 * @param UrdfFilePath - The path to the URDF file to read.
	 * @param UrdfPackagePath - The path to the package directory. Corresponds to the `package://`
	 * part of any filepath in the .urdf file, typically used for pointing at mesh files. Can be
	 * left empty if the URDF file does not have any file paths in it.
	 * @return An Actor containing the read objects.
	 */
	static AActor* ImportURDF(const FString& UrdfFilePath, const FString& UrdfPackagePath = "");
};
