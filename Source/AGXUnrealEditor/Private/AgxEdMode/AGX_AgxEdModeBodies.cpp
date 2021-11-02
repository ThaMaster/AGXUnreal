#include "AgxEdMode/AGX_AgxEdModeBodies.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"

// Unreal Engine includes.
#include "Textures/SlateIcon.h"

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

FSlateIcon UAGX_AgxEdModeBodies::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIcon,
		FAGX_EditorStyle::AgxIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
