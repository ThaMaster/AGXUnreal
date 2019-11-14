#include "AGXArchiveCommands.h"

#define LOCTEXT_NAMESPACE "FAGXArchiveModule"

void FAGXArchiveCommands::RegisterCommands()
{
	UI_COMMAND(ImportAction, "ImportAGXArchive", "Execute ImportAGXArchive action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportAction, "ExportAGXArchive", "Execute ExportAGXArchive action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
