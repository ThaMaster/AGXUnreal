// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternBase.h"

#include "AGX_RayPatternHorizontalSweep.generated.h"

/**
 * Scans one vertical line, then goes to the next.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_RayPatternHorizontalSweep : public UAGX_RayPatternBase
{
	GENERATED_BODY()

public:
	/**
	 * Field of View (FOV) of the Lidar Sensor [deg].
	 * The first element is the horizontal FOV, and the second vertical FOV.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector2D FOV {360.0, 40.0};

	/**
	 * Resolution, the smallest angle between two laser rays [deg].
	 * The first element is the horizontal FOV, and the second is vertical FOV.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector2D Resolution {1.0, 1.0};

	/**
	 * Determines how many times the total scan cycle is run per second, i.e. how often the total
	 * ray pattern of the Lidar Sensor is covered per second [Hz].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double Frequency {1.0};
};
