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
	double TileOverlap {500.0};

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Pager Settings")
	double TileSize {2500.0};

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Pager Settings")
	bool bDrawDebugGrid {true};

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Pager Settings")
	bool bDrawDebugLoadRadii {true};
};
