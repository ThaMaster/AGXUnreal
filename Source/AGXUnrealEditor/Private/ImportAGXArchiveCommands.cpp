#include "ImportAGXArchiveCommands.h"

#define LOCTEXT_NAMESPACE "FImportAGXArchiveModule"

void FImportAGXArchiveCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ImportAGXArchive", "Execute ImportAGXArchive action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
