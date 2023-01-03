// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"

// Unreal Engine includes.
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_HeightFieldBoundsComponentVisualizer"

namespace AGX_HeightFieldBoundsComponentVisualizer_helpers
{
	void DrawBounds(
		const FTransform& Transform, const FVector& HalfExtent, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const FBox BoundingBox(-HalfExtent, HalfExtent);
		DrawWireBox(
			PDI, Transform.ToMatrixNoScale(), BoundingBox, Color, SDPG_Foreground, LineThickness,
			0.f, true);
	}
}

void FAGX_HeightFieldBoundsComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_HeightFieldBoundsComponentVisualizer_helpers;
	const UAGX_HeightFieldBoundsComponent* Bounds =
		Cast<const UAGX_HeightFieldBoundsComponent>(Component);
	if (Bounds == nullptr)
		return;

	if (auto BoundsInfo = Bounds->GetUserSetBounds())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Gray, 2.f, PDI);
	}

	if (auto BoundsInfo = Bounds->GetLandscapeAdjustedBounds())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Green, 4.f, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
