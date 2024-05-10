// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputBase.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarOutputBarrier.h"

bool FAGX_LidarOutputBase::AddTo(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	return Lidar->AddResult(*this);
}

FAGX_LidarOutputBase& FAGX_LidarOutputBase::operator=(const FAGX_LidarOutputBase& Other)
{
	return *this;
}

bool FAGX_LidarOutputBase::operator==(const FAGX_LidarOutputBase& Other) const
{
	return HasNative() && Other.HasNative();
}
