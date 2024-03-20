// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

// Lidar Sensor Component Enums.

/** Specifies if the Lidar scan pattern used. */
UENUM()
enum class EAGX_LidarScanPattern
{
	/** Scans one vertical line, then goes to the next. */
	HorizontalSweep,

	/** Use a custom scan pattern. Ray transforms are set by the user. */
	Custom
};


// Lidar Sensor Line Trace Component Enums.

/** Specifies if the Lidar is run continuously or waits for a scan command. */
UENUM()
enum class EAGX_LidarExecutonMode
{
	/** The Lidar runs continously acoording to the specified frequency. */
	Auto,

	/** The Lidar is passive until an explicit scan command is given. */
	Manual
};

/** Specifies if the Lidar scan pattern used. */
UENUM()
enum class EAGX_LidarLineTraceScanPattern
{
	/** Scans one vertical line, then goes to the next. */
	HorizontalSweep,

	/** Scans one horizontal line, then goes to the next. */
	VerticalSweep
};
