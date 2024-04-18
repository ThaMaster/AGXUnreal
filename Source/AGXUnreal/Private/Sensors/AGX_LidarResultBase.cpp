// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarResultBase.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarResultBarrier.h"

bool FAGX_LidarResultBase::AddTo(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	return Lidar->AddResult(*this);
}

void FAGX_LidarResultBase::PostAllocateNative(FLidarResultBarrier* Native)
{
	if (Native == nullptr || !Native->HasNative())
		return;

	
	if (bEnableDistanceGaussianNoise)
	{
		Native->EnableDistanceGaussianNoise(
			DistanceNoiseSettings.Mean, DistanceNoiseSettings.StandardDeviation,
			DistanceNoiseSettings.StandardDeviationSlope);
	}
	else
	{
		Native->DisableDistanceGaussianNoise();
	}

	Native->EnableRemovePointsMisses(bEnableRemovePointsMisses);
}
