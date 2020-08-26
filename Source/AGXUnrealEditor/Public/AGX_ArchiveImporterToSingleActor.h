#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AActor;
class FString;

class AGXUNREALEDITOR_API AGX_ArchiveImporterToSingleActor
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
};
