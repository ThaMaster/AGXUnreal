#pragma once

#include "CoreMinimal.h"
#include "AGXArchiveStyle.h"
#include "Framework/Commands/Commands.h"

class FAGXArchiveCommands : public TCommands<FAGXArchiveCommands>
{
public:
	FAGXArchiveCommands()
		: TCommands<FAGXArchiveCommands>(TEXT("AGXArchive_Commands"),
			  NSLOCTEXT("Contexts", "AGXArchive_NSLOCTEXT", "AGXArchive_Plugin"), NAME_None,
			  FAGXArchiveStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportAction;
	TSharedPtr<FUICommandInfo> ExportAction;
};
