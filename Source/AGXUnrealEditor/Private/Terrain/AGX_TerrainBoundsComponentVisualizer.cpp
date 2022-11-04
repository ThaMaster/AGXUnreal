// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainBoundsComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_TerrainBoundsComponent.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainBoundsComponentVisualizer"

namespace AGX_TerrainBoundsComponentVisualizer_helpers
{
	void DrawBounds(
		const FTransform& Transform, const FVector& HalfExtent,
		const FLinearColor& Color, float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const FVector Corner0 = Transform.TransformPositionNoScale(
			FVector(-HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z));
		const FVector Corner1 =
			Transform.TransformPositionNoScale(FVector(HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z));
		const FVector Corner2 =
			Transform.TransformPositionNoScale(FVector(-HalfExtent.X, -HalfExtent.Y, HalfExtent.Z));
		const FVector Corner3 =
			Transform.TransformPositionNoScale(FVector(HalfExtent.X, -HalfExtent.Y, HalfExtent.Z));
		const FVector Corner4 =
			Transform.TransformPositionNoScale(FVector(-HalfExtent.X, HalfExtent.Y, -HalfExtent.Z));
		const FVector Corner5 =
			Transform.TransformPositionNoScale(FVector(HalfExtent.X, HalfExtent.Y, -HalfExtent.Z));
		const FVector Corner6 =
			Transform.TransformPositionNoScale(FVector(-HalfExtent.X, HalfExtent.Y, HalfExtent.Z));
		const FVector Corner7 =
			Transform.TransformPositionNoScale(FVector(HalfExtent.X, HalfExtent.Y, HalfExtent.Z));

		PDI->DrawLine(Corner0, Corner1, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner0, Corner2, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner2, Corner3, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner1, Corner3, Color, SDPG_Foreground, LineThickness, 0.f, true);

		PDI->DrawLine(Corner0, Corner4, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner1, Corner5, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner2, Corner6, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner3, Corner7, Color, SDPG_Foreground, LineThickness, 0.f, true);

		PDI->DrawLine(Corner4, Corner5, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner4, Corner6, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner6, Corner7, Color, SDPG_Foreground, LineThickness, 0.f, true);
		PDI->DrawLine(Corner5, Corner7, Color, SDPG_Foreground, LineThickness, 0.f, true);
	}
}

void FAGX_TerrainBoundsComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_TerrainBoundsComponentVisualizer_helpers;
	const UAGX_TerrainBoundsComponent* Bounds = Cast<const UAGX_TerrainBoundsComponent>(Component);
	if (Bounds == nullptr)
		return;

	if (auto BoundsInfo = Bounds->GetUserSetBounds())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Gray, 2.f, PDI);
	}

	static constexpr FColor Orange {255, 205, 14};
	if (auto BoundsInfo = Bounds->GetLandscapeAdjustedBounds())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, Orange, 4.f, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
