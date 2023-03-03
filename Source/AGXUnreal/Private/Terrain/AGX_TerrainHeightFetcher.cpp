#include "Terrain/AGX_TerrainHeightFetcher.h"

void FAGX_TerrainHeightFetcher::FetchHeights() const
{
	UE_LOG(LogAGX, Warning, TEXT("FAGX_TerrainHeightFetcher::FetchHeights() called..."));
}

void FAGX_TerrainHeightFetcher::SetTerrain(AAGX_Terrain* InTerrain)
{
	Terrain = InTerrain;
}

AAGX_Terrain* FAGX_TerrainHeightFetcher::GetTerrain() const
{
	return Terrain;
}
