#pragma once

#include "CoreMinimal.h"
#include "ImportAGXArchiveStyle.h"
#include "Framework/Commands/Commands.h"

class FImportAGXArchiveCommands : public TCommands<FImportAGXArchiveCommands>
{
public:
	FImportAGXArchiveCommands()
		: TCommands<FImportAGXArchiveCommands>(TEXT("ImportAGXArchive_Commands"),
			  NSLOCTEXT("Contexts", "ImportAGXArchive_NSLOCTEXT", "ImportAGXArchive_Plugin"), NAME_None,
			  FImportAGXArchiveStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};
