#pragma once

#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_ShapeUtilities
{
public:
	static bool ComputeOrientedBox(
		const TArray<FVector>& Vertices, FVector& OutHalfExtents, FTransform& OutTransform);
};
