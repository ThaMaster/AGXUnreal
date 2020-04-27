#pragma once

class UBlueprint;
class FString;

namespace AGX_ArchiveImporterToBlueprint
{
	/**
	 * Read simulation objects from the .agx archive pointed to by 'ArchivePath` and create an Actor
	 * Blueprint populated with corresponding AGXUnreal objects.
	 *
	 * @param ArchivePath - The path to the AGX Dynamics archive to read.
	 * @return An Actor Blueprint containing the read objects.
	 */
	UBlueprint* ImportAGXArchive(const FString& ArchivePath);
}
