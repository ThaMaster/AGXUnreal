#include "AGX_CustomVersion.h"

// Unreal Engine includes.
#include "Serialization/CustomVersion.h"

const FGuid FAGX_CustomVersion::GUID(0x9A157FFA, 0x909A4B69, 0xBE1B12A0, 0x51B6F233);

// Register the custom version with core
FCustomVersionRegistration GRegisterPaperCustomVersion(
	FAGX_CustomVersion::GUID, FAGX_CustomVersion::LatestVersion, TEXT("AGXUnrealVer"));
