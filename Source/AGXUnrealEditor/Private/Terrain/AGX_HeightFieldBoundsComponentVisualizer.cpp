// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_HeightFieldBoundsComponentVisualizer"

namespace AGX_HeightFieldBoundsComponentVisualizer_helpers
{
	void DrawRectangle(
		const FTransform& CenterTransform, double X, double Y, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const double Xh = X / 2.0;
		const double Yh = Y / 2.0;
		const FVector Corner0 = CenterTransform.TransformPositionNoScale(FVector(-Xh, -Yh, 0));
		const FVector Corner1 = CenterTransform.TransformPositionNoScale(FVector(-Xh, Yh, 0));
		const FVector Corner2 = CenterTransform.TransformPositionNoScale(FVector(Xh, Yh, 0));
		const FVector Corner3 = CenterTransform.TransformPositionNoScale(FVector(Xh, -Yh, 0));

		// We use a large DepthBias here so that the lines are drawn clearly and not dark
		// (underneeth the landscape).
		PDI->DrawLine(Corner0, Corner1, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner1, Corner2, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner2, Corner3, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner3, Corner0, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
	}

	void DrawTerrainPagerLoadedTiles(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		check(Terrain.bEnableTerrainPager);
		check(Terrain.SourceLandscape != nullptr);
		check(Terrain.HasNativeTerrainPager());

		const FTerrainPagerBarrier* TPBarrier = Terrain.GetNativeTerrainPager();
		if (TPBarrier == nullptr)
			return;

		const auto QuadSize = Terrain.SourceLandscape->GetActorScale().X;
		const int32 TileNumQuadsSide =
			FMath::RoundToInt32(Terrain.TerrainPagerSettings.TileSize / QuadSize);
		const double TileSize = QuadSize * TileNumQuadsSide;

		const FVector BoundsPosGlobal = BoundsTransform.GetLocation();

		for (auto TileTransform : TPBarrier->GetActiveTileTransforms())
		{
			const FVector TilePosGlobal = TileTransform.GetLocation();

			// We want to draw the tiles at the same height as the bounds to align everything. The
			// tiles may have a z-offset that depends on the first loaded tile average height (set
			// by AGX Dynamics automatically).
			TileTransform.SetLocation(FVector(TilePosGlobal.X, TilePosGlobal.Y, BoundsPosGlobal.Z));
			DrawRectangle(TileTransform, TileSize, -TileSize, FLinearColor::White, 8.f, PDI);
		}
	}

	void DrawTerrainPagerGrid(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		check(Terrain.bEnableTerrainPager);
		check(Terrain.SourceLandscape != nullptr);

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
				// The 0,0 Tile is actually extending in the x+ and y- direction in Unreal because
				// of AGX Dynamics coordinate system having flipped y-axis. That's why we negate the
				// y-directions.
				const double StartPosLocalX =
					static_cast<double>(X) * (TileSize - TileOverlap) + TileSize / 2.0;
				const double StartPosLocalY =
					static_cast<double>(Y) * (TileSize - TileOverlap) - TileSize / 2.0;
				const FVector StartPosGlobal = BoundsTransform.TransformPositionNoScale(
					FVector(StartPosLocalX, StartPosLocalY, 0.0));

				const FTransform RectangleTransform(BoundsTransform.GetRotation(), StartPosGlobal);
				DrawRectangle(
					RectangleTransform, TileSize, -TileSize, FLinearColor::Gray, 3.f, PDI);
			}
		}
	}

	void DrawTerrainPagerDebugRendering(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		if (!Terrain.bEnableTerrainPager)
			return;

		if (Terrain.SourceLandscape == nullptr)
			return;

		if (Terrain.TerrainPagerSettings.bDrawDebugGrid)
		{
			DrawTerrainPagerGrid(Terrain, BoundsTransform, HalfExtents, PDI);

			if (Terrain.HasNativeTerrainPager())
				DrawTerrainPagerLoadedTiles(Terrain, BoundsTransform, HalfExtents, PDI);
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
