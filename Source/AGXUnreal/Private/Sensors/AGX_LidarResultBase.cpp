// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarResultBase.h"

bool FAGX_LidarResultBase::AssociateWith(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	return Lidar->AddResult(*this);
}

