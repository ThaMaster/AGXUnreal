// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_SensorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_RayPatternCustom.h"
#include "Sensors/AGX_RayPatternHorizontalSweep.h"
#include "Sensors/LidarBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

FAGX_RealInterval FAGX_SensorUtilities::GetRangeFrom(EAGX_LidarModel InModel)
{
	if (InModel == EAGX_LidarModel::Custom)
	{
		return Cast<UAGX_LidarSensorComponent>(
				   UAGX_LidarSensorComponent::StaticClass()->GetDefaultObject())
			->Range;
	}

	return FLidarBarrier::GetRangeFrom(InModel);
}

FAGX_Real FAGX_SensorUtilities::GetBeamDivergenceFrom(EAGX_LidarModel InModel)
{
	if (InModel == EAGX_LidarModel::Custom)
	{
		return Cast<UAGX_LidarSensorComponent>(
				   UAGX_LidarSensorComponent::StaticClass()->GetDefaultObject())
			->BeamDivergence;
	}

	return FLidarBarrier::GetBeamDivergenceFrom(InModel);
}

FAGX_Real FAGX_SensorUtilities::GetBeamExitRadiusFrom(EAGX_LidarModel InModel)
{
	if (InModel == EAGX_LidarModel::Custom)
	{
		return Cast<UAGX_LidarSensorComponent>(
				   UAGX_LidarSensorComponent::StaticClass()->GetDefaultObject())
			->BeamExitRadius;
	}

	return FLidarBarrier::GetBeamExitRadiusFrom(InModel);
}

UAGX_RayPatternBase* FAGX_SensorUtilities::GetRayPatternFrom(EAGX_LidarModel InModel)
{
	switch (InModel)
	{
		case EAGX_LidarModel::Custom:
			return FAGX_ObjectUtilities::GetAssetFromPath<UAGX_RayPatternCustom>(
				TEXT("StaticMesh'/AGXUnreal/Sensor/Lidar/"
					 "AGX_RP_Custom.AGX_RP_Custom'"));
		case EAGX_LidarModel::GenericHorizontalSweep:
			return FAGX_ObjectUtilities::GetAssetFromPath<UAGX_RayPatternHorizontalSweep>(
				TEXT("StaticMesh'/AGXUnreal/Sensor/Lidar/"
					 "AGX_RP_HorizontalSweep.AGX_RP_HorizontalSweep'"));
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT("Unknown LidarModel passed to FAGX_SensorUtilities::GetRayPatternFrom, returning "
			 "nullptr."));
	return nullptr;
}

bool FAGX_SensorUtilities::GetEnableDistanceGaussianNoiseFrom(EAGX_LidarModel InModel)
{
	if (InModel == EAGX_LidarModel::Custom)
	{
		return Cast<UAGX_LidarSensorComponent>(
				   UAGX_LidarSensorComponent::StaticClass()->GetDefaultObject())
			->bEnableDistanceGaussianNoise;
	}

	return FLidarBarrier::GetEnableDistanceGaussianNoiseFrom(InModel);
}

FAGX_DistanceGaussianNoiseSettings FAGX_SensorUtilities::GetDistanceGaussianNoiseFrom(
	EAGX_LidarModel InModel)
{
	if (InModel == EAGX_LidarModel::Custom)
	{
		return Cast<UAGX_LidarSensorComponent>(
				   UAGX_LidarSensorComponent::StaticClass()->GetDefaultObject())
			->DistanceNoiseSettings;
	}

	auto DistanceNoise = FLidarBarrier::GetDistanceGaussianNoiseFrom(InModel);
	if (!DistanceNoise.IsSet())
		return FAGX_DistanceGaussianNoiseSettings();

	return DistanceNoise.GetValue();
}
