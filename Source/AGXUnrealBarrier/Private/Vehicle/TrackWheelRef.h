// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxVehicle/TrackWheel.h>
#include "EndAGXIncludes.h"

struct FTrackWheelRef
{
	agxVehicle::TrackWheelRef Native;

	FTrackWheelRef() = default;
	FTrackWheelRef(agxVehicle::TrackWheel* InNative)
		: Native(InNative)
	{
	}
};
