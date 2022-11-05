// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/HeightFieldShapeBarrier.h"

// Standard library includes.
#include <tuple>

class ALandscape;

namespace AGX_HeightFieldUtilities
{
	// StartPos is in world coordinate system.
	AGXUNREAL_API FHeightFieldShapeBarrier
	CreateHeightField(ALandscape& Landscape, const FVector& StartPos, float LengthX, float LengthY);

	// The resulting Transform's position will always intersect the Landscape, i.e. even in the
	// given Center point does not lie in the Landscape's plane, it will be projected to it during
	// the calculations.
	// The Center point must be given in global coordinate system.
	AGXUNREAL_API FTransform GetTerrainTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent);

	// Same as GetTerrainTransformUsingBoxFrom but for Height Field.
	AGXUNREAL_API FTransform GetHeightFieldTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent);
}
