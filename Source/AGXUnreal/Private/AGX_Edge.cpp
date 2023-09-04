// Copyright 2023, Algoryx Simulation AB.

#include "AGX_Edge.h"

FTwoVectors FAGX_Edge::GetLocationsRelativeTo(USceneComponent* Component)
{
	FTwoVectors Line;
	Line.v1 = Start.GetLocationRelativeTo(Component);
	Line.v2 = End.GetLocationRelativeTo(Component);
	return Line;
}
