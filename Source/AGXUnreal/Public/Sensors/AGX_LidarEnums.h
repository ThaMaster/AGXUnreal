// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

/** Specifies if the Lidar laser sampling is done on CPU or other types. */
UENUM()
enum class EAGX_LidarSamplingType
{
	/**
	 * The Lidar laser rays are handled on the CPU.
	 * Important: Terrain particles and deformations as well as primitive AGX Shapes will not be
	 * detected when using this mode.
	 */
	CPU
};

/** Specifies if the Lidar is run continuously or waits for a scan command. */
UENUM()
enum class EAGX_LidarExecutonMode
{
	/** The Lidar runs continously acoording to the specified frequency. */
	Auto,

	/** The Lidar is passive until an explicit scan command is given. */
	Manual,
};

/** Specifies if the Lidar scan pattern used. */
UENUM()
enum class EAGX_LidarScanPattern
{
	/** Scans one vertical line, then goes to the next. */
	VerticalSweep,

	/** Scans one horizontal line, then goes to the next. */
	HorizontalSweep,
};
