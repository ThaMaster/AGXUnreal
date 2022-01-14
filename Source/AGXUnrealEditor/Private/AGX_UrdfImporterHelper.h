// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "AGX_SimObjectsImporterHelper.h"

struct FAGX_UrdfImporterHelper : public FAGX_SimObjectsImporterHelper
{
	explicit FAGX_UrdfImporterHelper(const FString& InUrdfFilePath, const FString& InUrdfPackagePath);

	const FString UrdfPackagePath;
};
