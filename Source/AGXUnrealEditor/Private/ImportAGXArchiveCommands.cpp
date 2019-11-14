#include "ImportAGXArchiveCommands.h"

#define LOCTEXT_NAMESPACE "FImportAGXArchiveModule"

void FImportAGXArchiveCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ImportAGXArchive", "Execute ImportAGXArchive action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportAction, "ExportAGXArchive", "Execute ExportAGXArchive action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
