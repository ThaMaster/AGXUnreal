#pragma once


class UBlueprint;
class FString;

namespace AGX_ArchiveImporterToBlueprint
{
	UBlueprint* ImportAGXArchive(const FString& ArchivePath);
}
