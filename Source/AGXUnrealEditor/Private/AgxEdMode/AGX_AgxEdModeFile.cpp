// Fill out your copyright notice in the Description page of Project Settings.


#include "AgxEdMode/AGX_AgxEdModeFile.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeFile"


UAGX_AgxEdModeFile* UAGX_AgxEdModeFile::GetInstance()
{
	static UAGX_AgxEdModeFile* ConstraintTool = nullptr;

	if (ConstraintTool == nullptr)
	{
		ConstraintTool = GetMutableDefault<UAGX_AgxEdModeFile>();
	}

	return ConstraintTool;
}

FText UAGX_AgxEdModeFile::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "File");
}

FText UAGX_AgxEdModeFile::GetTooltip() const
{
	return LOCTEXT("Tooltip", "Interoperability with external file formats, such AGX simulation files (.agx)");
}

#undef LOCTEXT_NAMESPACE
