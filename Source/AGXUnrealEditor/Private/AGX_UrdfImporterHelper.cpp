// Copyright 2022, Algoryx Simulation AB.


#include "AGX_UrdfImporterHelper.h"

FAGX_UrdfImporterHelper::FAGX_UrdfImporterHelper(
	const FString& InUrdfFilePath, const FString& InUrdfPackagePath)
	: FAGX_SimObjectsImporterHelper(InUrdfFilePath)
	, UrdfPackagePath(InUrdfPackagePath)
{
}
