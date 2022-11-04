// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"

#include "AGX_LandscapeSizeInfo.generated.h"

USTRUCT()
struct FAGX_LandscapeSizeInfo
{
	GENERATED_USTRUCT_BODY()
	FAGX_LandscapeSizeInfo() = default;
	FAGX_LandscapeSizeInfo(const ALandscape& Landscape);
	static std::tuple<int32, int32> GetComponentsPerSide(const ALandscape& Landscape);
	static std::tuple<float, float> GetSideLengths(const ALandscape& Landscape);
	static bool IsOpenWorldLandscape(const ALandscape& Landscape);

	int32 NumComponents;
	int32 NumQuadsPerComponentSide;
	int32 NumVerticesPerSectionSide;
	int32 NumComponentsSideX;
	int32 NumComponentsSideY;
	int32 NumSectionsSideX;
	int32 NumSectionsSideY;
	int32 NumQuadsSideX;
	int32 NumQuadsSideY;
	int32 NumVertices;
	int32 NumVerticesSideX;
	int32 NumVerticesSideY;
	float QuadSideSizeX;
	float QuadSideSizeY;
	float LandscapeScaleZ;

	// This is guaranteed to be uniform (i.e. n X n sections per component).
	int32 NumSectionSidesPerComponentSide;
};
