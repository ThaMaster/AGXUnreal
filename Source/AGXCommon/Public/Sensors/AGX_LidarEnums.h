// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies the model of the Lidar. */
UENUM()
enum class EAGX_LidarModel
{
	/** Generic Lidar with a Horizontal Sweep ray pattern. */
	GenericHorizontalSweep,

	/** Lidar uses a custom ray pattern where the user provides the ray pattern rays. */
	Custom
};
