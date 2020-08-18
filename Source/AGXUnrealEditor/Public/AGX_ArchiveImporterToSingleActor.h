#pragma once

class AActor;
class FString;

namespace AGX_ArchiveImporterToSingleActor
{
	/**
	 * Read simulation objects from the .agx archive poitned to by 'ArchivePath' and create an Actor
	 * populated with corresponding AGXUnreal Component objects.
	 *
	 * @param ArchivePath - The path to the AGX Dynamics archive to read.
	 * @return An Actor containing Components for each object read.
	 */
	AGXUNREALEDITOR_API AActor* ImportAGXArchive(const FString& ArchivePath);
}
