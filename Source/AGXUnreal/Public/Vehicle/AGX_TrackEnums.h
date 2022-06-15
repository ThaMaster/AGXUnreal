// Authored by VMC Motion Technologies, 2022.


#pragma once

#include "CoreMinimal.h"

#include "AGX_TrackEnums.generated.h"


UENUM(BlueprintType)
// Unreal Header Tool does not support line breaks in UMETA tags.
// clang-format off
enum EAGX_TrackWheelModel
{
	TWM_SPROCKET UMETA(DisplayName = "Sprocket", ToolTip = "Geared driving wheel. Will merge track nodes to itself."),
	TWM_IDLER UMETA(DisplayName = "Idler", ToolTip = "Geared non-powered wheel. Will merge track nodes to itself."),
	TWM_ROLLER UMETA(DisplayName = "Roller", ToolTip = "Track return or road wheel.")
};
// clang-format on