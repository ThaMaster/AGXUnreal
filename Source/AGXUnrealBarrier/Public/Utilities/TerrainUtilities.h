// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class FTerrainBarrier;

struct FParticleData;

class FTerrainUtilities
{
public:
	// @todo: add description about array reserve etc.
	static void AppendParticlePositions(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions);

	static void AppendParticleRadii(const FTerrainBarrier& Terrain, TArray<float>& OutRadii);

	static void AppendParticleRotations(
		const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations);

	static void AppendParticleData(const FTerrainBarrier& Terrain, FParticleData& OutParticleData);

	static size_t GetNumParticles(const FTerrainBarrier& Terrain);
};
