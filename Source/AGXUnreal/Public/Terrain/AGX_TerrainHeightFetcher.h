// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainHeightFetcherBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class AAGX_Terrain;

class FAGX_TerrainHeightFetcher : public FTerrainHeightFetcherBase
{
public:
	virtual void FetchHeights() const override;

	void SetTerrain(AAGX_Terrain* Terrain);
	AAGX_Terrain* GetTerrain() const;

private:
	AAGX_Terrain* Terrain = nullptr;
};
