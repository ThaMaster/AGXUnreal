// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/TerrainDataSource.h>
#include "EndAGXIncludes.h"

class FTerrainHeightFetcherBase;

class FTerrainDataSource : public agxTerrain::TerrainDataSource
{
public:
	virtual agxTerrain::TerrainDataSource::TerrainHeightType fetchTerrainTile(
		const agxTerrain::TileSpecification& ts, agxTerrain::TileId id) override;

	void SetTerrainHeightFetcher(FTerrainHeightFetcherBase* HeightFetcher);
	FTerrainHeightFetcherBase* GetTerrainHeightFetcher() const;

private:
	FTerrainHeightFetcherBase* HeightFetcher = nullptr;
};
