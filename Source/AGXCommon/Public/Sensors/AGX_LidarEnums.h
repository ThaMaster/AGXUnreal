// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"


/** Specifies if the Lidar scan pattern used. */
UENUM()
enum class EAGX_LidarRayPattern
{
	Invalid,

	/** Scans one vertical line, then goes to the next. */
	HorizontalSweep,

	/** Use a custom scan pattern. Ray transforms are set by the user. */
	Custom
};
