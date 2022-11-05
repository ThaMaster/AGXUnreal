// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_LandscapeSizeInfo.h"

#include "LandscapeComponent.h"

#include <tuple>

FAGX_LandscapeSizeInfo::FAGX_LandscapeSizeInfo(const ALandscape& Landscape)
{
	NumComponents = Landscape.LandscapeComponents.Num();
	NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
	QuadSideSizeX = Landscape.GetActorScale().X;
	QuadSideSizeY = Landscape.GetActorScale().Y;
	NumSectionSidesPerComponentSide = Landscape.NumSubsections;
	NumVerticesPerSectionSide = Landscape.SubsectionSizeQuads + 1;
	LandscapeScaleZ = Landscape.GetActorScale3D().Z;
	std::tuple<int32, int32> ComponentsPerSide = GetComponentsPerSide(Landscape);
	NumComponentsSideX = std::get<0>(ComponentsPerSide);
	NumComponentsSideY = std::get<1>(ComponentsPerSide);
	NumSectionsSideX = NumComponentsSideX * NumSectionSidesPerComponentSide;
	NumSectionsSideY = NumComponentsSideY * NumSectionSidesPerComponentSide;
	NumQuadsSideX = NumComponentsSideX * NumQuadsPerComponentSide;
	NumQuadsSideY = NumComponentsSideY * NumQuadsPerComponentSide;
	NumVerticesSideX = NumQuadsSideX + 1;
	NumVerticesSideY = NumQuadsSideY + 1;
	NumVertices = NumVerticesSideX * NumVerticesSideY;
}

std::tuple<int32, int32> FAGX_LandscapeSizeInfo::GetComponentsPerSide(const ALandscape& Landscape)
{
	TArray<int32> ComponentUniqueSectionBaseX;
	TArray<int32> ComponentUniqueSectionBaseY;

	for (int i = 0; i < Landscape.LandscapeComponents.Num(); i++)
	{
		ComponentUniqueSectionBaseX.AddUnique(Landscape.LandscapeComponents[i]->SectionBaseX);
		ComponentUniqueSectionBaseY.AddUnique(Landscape.LandscapeComponents[i]->SectionBaseY);
	}

	const int32 ComponentsSideX = ComponentUniqueSectionBaseX.Num();
	const int32 ComponentsSideY = ComponentUniqueSectionBaseY.Num();

	return std::tuple<int32, int32> {ComponentsSideX, ComponentsSideY};
}

std::tuple<float, float> FAGX_LandscapeSizeInfo::GetSideLengths(const ALandscape& Landscape)
{
	std::tuple<int32, int32> ComponentsPerSide = GetComponentsPerSide(Landscape);
	const int32 NumComponentsSideX = std::get<0>(ComponentsPerSide);
	const int32 NumComponentsSideY = std::get<1>(ComponentsPerSide);
	const int32 NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
	const int32 NumQuadsSideX = NumComponentsSideX * NumQuadsPerComponentSide;
	const int32 NumQuadsSideY = NumComponentsSideY * NumQuadsPerComponentSide;
	const float QuadSideSizeX = Landscape.GetActorScale().X;
	const float QuadSideSizeY = Landscape.GetActorScale().Y;
	return std::make_tuple<float, float>(
		static_cast<float>(NumQuadsSideX) * QuadSideSizeX,
		static_cast<float>(NumQuadsSideY) * QuadSideSizeY);
}

bool FAGX_LandscapeSizeInfo::IsOpenWorldLandscape(const ALandscape& Landscape)
{
	// This is just an observation that holds true for OpenWorldLandscapes, would be better
	// with a more "correct" way of determining this.
	return Landscape.LandscapeComponents.Num() == 0;
}