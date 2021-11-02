#include "AgxEdMode/AGX_AgxEdModeDebug.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"

// Unreal Engine includes.
#include "Textures/SlateIcon.h"

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

FSlateIcon UAGX_AgxEdModeDebug::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIcon,
		FAGX_EditorStyle::AgxIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
