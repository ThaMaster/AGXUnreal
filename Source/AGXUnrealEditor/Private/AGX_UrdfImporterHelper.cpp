#include "AGX_UrdfImporterHelper.h"

FAGX_UrdfImporterHelper::FAGX_UrdfImporterHelper(
	const FString& InUrdfFilePath, const FString& InUrdfPackagePath)
	: FAGX_ArchiveImporterHelper(InUrdfFilePath, true)
	, UrdfPackagePath(InUrdfPackagePath)
{
}
