// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class FTerrainBarrier;

struct FParticleData;
struct FParticleDataById;

class FTerrainUtilities
{
public:
	/**
	 * Writes the position of all particles known to the passed Terrain to OutPositions.
	 */
	static void AppendParticlePositions(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions);


	static void AppendParticleVelocities(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities);

	/**
	 * Writes the radii of all particles known to the passed Terrain to OutRadii.
	 */
	static void AppendParticleRadii(const FTerrainBarrier& Terrain, TArray<float>& OutRadii);

	/**
	 * Writes the rotation of all particles known to the passed Terrain to OutRotations.
	 */
	static void AppendParticleRotations(
		const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations);

	/**
	 * Writes the position, rotation and radii of all particles known to the passed Terrain to OutParticleData.
	 */
	static void AppendParticleData(const FTerrainBarrier& Terrain, FParticleData& OutParticleData);

	static void GetParticleExistsById(const FTerrainBarrier& Terrain, TArray<bool>& OutExists);

	static void GetParticlePositionsById(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions);

	static void GetParticleVelocitiesById(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities);

	static void GetParticleRotationsById(
		const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotation);

	static void GetParticleRadiiById(
		const FTerrainBarrier& Terrain, TArray<float>& OutRadii);

	/**
	 * Writes the position, rotation and radii of all particles known to the passed Terrain to OutParticleData.
	 */
	static void GetParticleDataById(const FTerrainBarrier& Terrain, FParticleDataById& OutParticleData);

	/**
	 * Returns the number of particles known to the passed Terrain.
	 */
	static size_t GetNumParticles(const FTerrainBarrier& Terrain);
};
