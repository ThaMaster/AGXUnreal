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

	AGXUNREAL_API std::tuple<FVector, FQuat> GetTerrainPositionAndRotationFrom(
		const ALandscape& Landscape);

	AGXUNREAL_API std::tuple<FVector, FQuat> GetHeightFieldPositionAndRotationFrom(
		const ALandscape& Landscape);
}
