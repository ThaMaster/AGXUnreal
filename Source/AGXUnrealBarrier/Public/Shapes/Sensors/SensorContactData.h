#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/Sensors/SensorContactPoint.h"

// Unreal Engine includes.
#include "Containers/Array.h"

struct AGXUNREALBARRIER_API FSensorContactData
{
	float dummy;
	TArray<FSensorContactPoint> Points;
};
