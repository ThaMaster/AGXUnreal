// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_HeightFieldBoundsComponentVisualizer"

namespace AGX_HeightFieldBoundsComponentVisualizer_helpers
{
	void DrawRectangle(
		const FTransform& CornerTransform, double X, double Y, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const FVector Corner0 = CornerTransform.GetLocation();
		const FVector Corner1 = CornerTransform.TransformPositionNoScale(FVector(0, Y, 0));
		const FVector Corner2 = CornerTransform.TransformPositionNoScale(FVector(X, Y, 0));
		const FVector Corner3 = CornerTransform.TransformPositionNoScale(FVector(X, 0, 0));

		PDI->DrawLine(Corner0, Corner1, Color, SDPG_Foreground);
		PDI->DrawLine(Corner1, Corner2, Color, SDPG_Foreground);
		PDI->DrawLine(Corner2, Corner3, Color, SDPG_Foreground);
		PDI->DrawLine(Corner3, Corner0, Color, SDPG_Foreground);
	}

	void DrawTerrainPagerDebugRendering(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		if (!Terrain.bEnableTerrainPager)
			return;

		if (Terrain.SourceLandscape == nullptr)
			return;

		const auto QuadSize = Terrain.SourceLandscape->GetActorScale().X;
		const int32 TileNumQuadsSide =
			FMath::RoundToInt32(Terrain.TerrainPagerSettings.TileSize / QuadSize);
		const int32 TileOverlapNumQuads =
			FMath::RoundToInt32(Terrain.TerrainPagerSettings.TileOverlap / QuadSize);

		const double TileSize = QuadSize * TileNumQuadsSide;
		const double TileOverlap = QuadSize * TileOverlapNumQuads;
		const double Tolerance = QuadSize / 1000.0;

		// p and n postfixes are for positive direction and negative direction respectively.
		// In the positive x and negative y directions, the length span of the tiles are given by
		// n * TileSize - (n - 1) * TileOverlap, and for the other directions given by
		// n * (TileSize - Overlap). This is due to the placement of the TerrainPager reference
		// position in relation to the Tiles.
		int32 NumTilesXp = static_cast<int32>(HalfExtents.X / (TileSize - TileOverlap));
		if (NumTilesXp * TileSize - (NumTilesXp - 1) * TileOverlap > HalfExtents.X + Tolerance)
		{
			// We overshot by one Tile (see comment above).
			NumTilesXp--;
		}

		int32 NumTilesXn = static_cast<int32>(HalfExtents.X / (TileSize - TileOverlap));

		int32 NumTilesYn = static_cast<int32>(HalfExtents.Y / (TileSize - TileOverlap));
		if (NumTilesYn * TileSize - (NumTilesYn - 1) * TileOverlap > HalfExtents.Y + Tolerance)
		{
			// We overshot by one Tile (see comment above).
			NumTilesYn--;
		}

		int32 NumTilesYp = static_cast<int32>(HalfExtents.Y / (TileSize - TileOverlap));

		for (int32 Y = NumTilesYp; Y > -NumTilesYn; Y--)
		{
			for (int32 X = -NumTilesXn; X < NumTilesXp; X++)
			{
				const double StartPosLocalX = static_cast<double>(X) * (TileSize - TileOverlap);
				const double StartPosLocalY = static_cast<double>(Y) * (TileSize - TileOverlap);
				const FVector StartPosGlobal = BoundsTransform.TransformPositionNoScale(
					FVector(StartPosLocalX, StartPosLocalY, 0.0));

				const FTransform RectangleTransform(BoundsTransform.GetRotation(), StartPosGlobal);
				DrawRectangle(
					RectangleTransform, TileSize, -TileSize, FLinearColor::Gray, 2.f, PDI);
			}
		}
	}

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

	auto BoundsInfo = Bounds->GetLandscapeAdjustedBounds();
	if (BoundsInfo.IsSet())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Green, 4.f, PDI);
	}

	// If part of a Terrain using Terrain Paging, draw debug rendering for it.
	AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(Bounds->GetOwner());
	if (BoundsInfo.IsSet() && Terrain != nullptr && Terrain->bEnableTerrainPager)
	{
		DrawTerrainPagerDebugRendering(
			*Terrain, BoundsInfo->Transform, BoundsInfo->HalfExtent, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
