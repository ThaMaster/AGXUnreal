// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_SensorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_RayPatternBase.h"
#include "AGX_SensorUtilities.h"


FAGX_RealInterval FAGX_SensorUtilities::GetRangeFrom(EAGX_LidarModel InModel)
{
	return FAGX_RealInterval();
}

FAGX_Real FAGX_SensorUtilities::GetBeamDivergenceFrom(EAGX_LidarModel InModel)
{
	return FAGX_Real();
}

FAGX_Real FAGX_SensorUtilities::GetBeamExitRadiusFrom(EAGX_LidarModel InModel)
{
	return FAGX_Real();
}

UAGX_RayPatternBase* FAGX_SensorUtilities::GetRayPatternFrom(EAGX_LidarModel InModel)
{
	return nullptr;
}

bool FAGX_SensorUtilities::GetEnableDistanceGaussianNoiseFrom(EAGX_LidarModel InModel)
{
	return false;
}

FAGX_DistanceGaussianNoiseSettings FAGX_SensorUtilities::GetDistanceGaussianNoiseFrom(
	EAGX_LidarModel InModel)
{
	return FAGX_DistanceGaussianNoiseSettings();
}
