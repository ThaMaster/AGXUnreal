// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainPagerSettings.generated.h"

USTRUCT()
struct FAGX_TerrainPagerSettings
{
	GENERATED_USTRUCT_BODY()

	// @todo add API comments

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Pager Settings")
	int TileOverlap = 500.f;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Pager Settings")
	int TileSize = 3000.f;
};
