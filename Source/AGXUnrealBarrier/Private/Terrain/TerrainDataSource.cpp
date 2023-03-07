#include "Terrain/TerrainDataSource.h"

using namespace agxTerrain;

TerrainDataSource::TerrainHeightType FTerrainDataSource::fetchTerrainTile(
	const TileSpecification& ts, TileId id)
{
	UE_LOG(LogTemp, Warning, TEXT("FTerrainDataSource::fetchTerrainTile called!"));
	return TerrainHeightType();
}

void FTerrainDataSource::SetTerrainHeightFetcher(FTerrainHeightFetcherBase* InHeightFetcher)
{
	HeightFetcher = InHeightFetcher;
}


FTerrainHeightFetcherBase* FTerrainDataSource::GetTerrainHeightFetcher() const
{
	return HeightFetcher;
}
