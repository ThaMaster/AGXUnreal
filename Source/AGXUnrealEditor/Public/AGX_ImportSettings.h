// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ImportEnums.h"

struct FAGX_ImportSettings
{
	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;
	FString FilePath;
	bool bIgnoreDisabledTrimeshes = true;
	bool bOpenBlueprintEditorAfterImport = true;

	// The path to the URDF package directory. Corresponds to the `package://` part of any filepath
	// in the .urdf file, typically used for pointing at mesh files. Can be left empty if the URDF
	// file does not have any file paths in it, or obviously, if ImportType is not Urdf.
	FString UrdfPackagePath;
};
