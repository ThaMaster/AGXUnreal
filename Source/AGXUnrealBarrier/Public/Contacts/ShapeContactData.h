#pragma once

#if 0
// AGX Dynamics for Unreal includes.
#include "Contacts/ShapeContactPoint.h"

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
#endif
