// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/HeightFieldShapeBarrier.h"

// Standard library includes.
#include <tuple>

class ALandscape;

namespace AGX_HeightFieldUtilities
{
	// StartPos is in world coordinate system.
	AGXUNREAL_API FHeightFieldShapeBarrier CreateHeightField(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY,
		bool ReadInitialHeights = true);

	// The resulting Transform's position will always intersect the Landscape, i.e. even in the
	// given Center point does not lie in the Landscape's plane, it will be projected to it during
	// the calculations.
	// The Center point must be given in global coordinate system.
	AGXUNREAL_API FTransform GetTerrainTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent);

	// Same as GetTerrainTransformUsingBoxFrom but for Height Field.
	AGXUNREAL_API FTransform GetHeightFieldTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent);

	// Overall resolution using outer bounds (i.e. holes does not affect this value unless a
	// complete part if a side has been removed using the Landscape tool.
	AGXUNREAL_API std::tuple<int32, int32> GetLandscapeNumberOfVertsXY(const ALandscape& Landscape);

	// Size (outer bounds) [cm].
	AGXUNREAL_API std::tuple<double, double> GetLandscapeSizeXY(const ALandscape& Landscape);

	AGXUNREAL_API bool IsOpenWorldLandscape(const ALandscape& Landscape);
}
