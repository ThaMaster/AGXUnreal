#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/Sensors/SensorContactPoint.h"

// Unreal Engine includes.
#include "Containers/Array.h"

struct AGXUNREALBARRIER_API FSensorContactData
{
	FGuid FirstShapeGuid;
	FGuid SecondShapeGuid;
	FGuid FirstBodyGuid;
	FGuid SecondBodyGuid;
	TArray<FSensorContactPoint> Points;
};
