#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/Contacts/ShapeContactPoint.h"

// Unreal Engine includes.
#include "Containers/Array.h"

struct AGXUNREALBARRIER_API FShapeContactData
{
	FGuid FirstShapeGuid;
	FGuid SecondShapeGuid;
	FGuid FirstBodyGuid;
	FGuid SecondBodyGuid;
	TArray<FShapeContactPoint> Points;
};
