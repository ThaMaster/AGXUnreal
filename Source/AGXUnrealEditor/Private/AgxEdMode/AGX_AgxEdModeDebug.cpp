#include "AgxEdMode/AGX_AgxEdModeDebug.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeDebug"

UAGX_AgxEdModeDebug* UAGX_AgxEdModeDebug::GetInstance()
{
	static UAGX_AgxEdModeDebug* ConstraintTool = nullptr;

	if (ConstraintTool == nullptr)
	{
		ConstraintTool = GetMutableDefault<UAGX_AgxEdModeDebug>();
	}

	return ConstraintTool;
}

FText UAGX_AgxEdModeDebug::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Debug");
}

FText UAGX_AgxEdModeDebug::GetTooltip() const
{
	return LOCTEXT("Tooltip", "Contains tools for debugging, logging, visualization, etc");
}

#undef LOCTEXT_NAMESPACE
