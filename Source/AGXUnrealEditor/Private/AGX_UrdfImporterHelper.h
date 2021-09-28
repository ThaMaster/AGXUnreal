#pragma once

#include "AGX_ArchiveImporterHelper.h"

struct FAGX_UrdfImporterHelper : public FAGX_ArchiveImporterHelper
{
	explicit FAGX_UrdfImporterHelper(const FString& InUrdfFilePath, const FString& InUrdfPackagePath);

	const FString UrdfPackagePath;
};
