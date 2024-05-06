// Copyright 2024, Algoryx Simulation AB.

#include "AGX_Edge.h"

FTwoVectors FAGX_Edge::GetLocationsRelativeTo(const USceneComponent& Component, const AActor* LocalScope) const
{
	FTwoVectors Line;
	Line.v1 = Start.GetLocationRelativeTo(Component, LocalScope);
	Line.v2 = End.GetLocationRelativeTo(Component, LocalScope);
	return Line;
}

FTwoVectors FAGX_Edge::GetLocationsRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent, const AActor* LocalScope) const
{
	FTwoVectors Line;
	Line.v1 = Start.GetLocationRelativeTo(Component, FallbackParent, LocalScope);
	Line.v2 = End.GetLocationRelativeTo(Component, FallbackParent, LocalScope);
	return Line;
}
