// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarModelParameters.h"

#include "AGX_OusterOS0Parameters.generated.h"

/**
 * Parameters used to create a Lidar Sensor Component using Lidar Model OusterOS0.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXCOMMON_API UAGX_OusterOS0Parameters : public UAGX_LidarModelParameters
{
	GENERATED_BODY()

public:
	/** Number of channels setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSChannelCount ChannelCount {EAGX_OusterOSChannelCount::Ch_32};

	/** Channel distribution setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSChannelDistribution ChannelDistribution {
		EAGX_OusterOSChannelDistribution::Uniform};

	/** Horizontal resolution setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSHorizontalResolution HorizontalResolution {
		EAGX_OusterOSHorizontalResolution::Hr_512};

	/** Frequency setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSFrequency Frequency {EAGX_OusterOSFrequency::F_10};
};
