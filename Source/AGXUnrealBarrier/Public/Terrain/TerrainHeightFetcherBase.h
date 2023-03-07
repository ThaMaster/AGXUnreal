// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FTerrainHeightFetcherBase
{
public:

	// @todo: document behavior.
	virtual bool FetchHeights(
		const FVector& WorldPosStart, int32 VertsX, int32 VertsY,
		TArray<float>& OutHeights) const = 0;
};
