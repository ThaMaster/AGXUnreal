// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_LandscapeSizeInfo.h"

#include "LandscapeComponent.h"

#include <tuple>

namespace
{
	// \todo find proper way of getting the landscape components per side count. This is a bit of a
	// hack.
	std::tuple<int32, int32> GetComponentsPerSide(ALandscape& Landscape)
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

		check(ComponentsSideX * ComponentsSideY == Landscape.LandscapeComponents.Num());

		return std::tuple<int32, int32> {ComponentsSideX, ComponentsSideY};
	}
}

FAGX_LandscapeSizeInfo::FAGX_LandscapeSizeInfo(ALandscape& Landscape)
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
