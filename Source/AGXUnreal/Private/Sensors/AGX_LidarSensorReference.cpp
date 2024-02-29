// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorReference.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

FAGX_LidarSensorReference::FAGX_LidarSensorReference()
	: FAGX_ComponentReference(UAGX_LidarSensorComponent::StaticClass())
{
}

UAGX_LidarSensorComponent* FAGX_LidarSensorReference::GetLidarComponent() const
{
	return Super::GetComponent<UAGX_LidarSensorComponent>();
}
