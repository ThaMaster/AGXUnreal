// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"

#include "AGX_TrackEnums.generated.h"


// Unreal Header Tool does not support line breaks in UMETA tags.
// clang-format off
/**
 * The different types of wheels supported by AGX Dynamics.
 *
 * The values of these must match the corresponding enum in AGX Dynamics.
 */
UENUM(BlueprintType)
enum class EAGX_TrackWheelModel : uint8
{
	TWM_SPROCKET UMETA(DisplayName = "Sprocket", ToolTip = "Geared driving wheel. Will merge track nodes to itself."),
	TWM_IDLER UMETA(DisplayName = "Idler", ToolTip = "Geared non-powered wheel. Will merge track nodes to itself."),
	TWM_ROLLER UMETA(DisplayName = "Roller", ToolTip = "Track return or road wheel.")
};
// clang-format on
