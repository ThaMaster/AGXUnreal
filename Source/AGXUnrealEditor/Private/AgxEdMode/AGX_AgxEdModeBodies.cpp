#include "AgxEdMode/AGX_AgxEdModeBodies.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeBodies"

UAGX_AgxEdModeBodies* UAGX_AgxEdModeBodies::GetInstance()
{
	static UAGX_AgxEdModeBodies* ConstraintTool = nullptr;

	if (ConstraintTool == nullptr)
	{
		ConstraintTool = GetMutableDefault<UAGX_AgxEdModeBodies>();
	}

	return ConstraintTool;
}

FText UAGX_AgxEdModeBodies::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Bodies");
}

FText UAGX_AgxEdModeBodies::GetTooltip() const
{
	return LOCTEXT(
		"Tooltip", "Contains tools to quickly create and manage AGX Rigid Bodies and Shapes");
}

#undef LOCTEXT_NAMESPACE
