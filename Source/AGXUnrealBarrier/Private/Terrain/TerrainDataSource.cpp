#include "Terrain/TerrainDataSource.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainHeightFetcherBase.h"

using namespace agxTerrain;

TerrainDataSource::TerrainHeightType FTerrainDataSource::fetchTerrainTile(
	const TileSpecification& ts, TileId id)
{
	if (HeightFetcher == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("fetchTerrainTile called on a TerrainDataSource that does not have a "
				 "HeightFetcher set. Fetchin Terrain Tile will fail."));
		return TerrainHeightType();
	}

	const agx::Real TileSizeHalfAGX = ts.getTileSize() / 2.0;
	const FVector TileCornerWorldPosUnreal = ConvertDisplacement(
		ts.convertTilePositionToWorld(id, agx::Vec2(-TileSizeHalfAGX, TileSizeHalfAGX)));
	const int32 VertCountSide = static_cast<int32>(ts.getTileResolution());

	TArray<float> Heights;
	const bool Result = HeightFetcher->FetchHeights(
		TileCornerWorldPosUnreal, VertCountSide, VertCountSide, Heights);

	if (!Result)
	{
		TerrainHeightType();
	}

	TerrainHeightType HeightsAGX;
	HeightsAGX.reserve(Heights.Num());
	for (float Height : Heights)
	{
		HeightsAGX.push_back(ConvertDistanceToAGX(Height));
	}

	return HeightsAGX;
}

void FTerrainDataSource::SetTerrainHeightFetcher(FTerrainHeightFetcherBase* InHeightFetcher)
{
	HeightFetcher = InHeightFetcher;
}

FTerrainHeightFetcherBase* FTerrainDataSource::GetTerrainHeightFetcher() const
{
	return HeightFetcher;
}
