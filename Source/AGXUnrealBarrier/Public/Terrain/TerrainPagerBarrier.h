// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <memory>

class FShovelBarrier;
class FTerrainBarrier;
class FTerrainHeightFetcherBase;

struct FParticleData;
struct FTerrainDataSourceRef;
struct FTerrainPagerRef;

class AGXUNREALBARRIER_API FTerrainPagerBarrier
{
public:
	FTerrainPagerBarrier();
	FTerrainPagerBarrier(std::unique_ptr<FTerrainPagerRef> InNativeRef);
	FTerrainPagerBarrier(FTerrainPagerBarrier&& Other);
	~FTerrainPagerBarrier();

	bool HasNative() const;
	void AllocateNative(
		FTerrainHeightFetcherBase* HeightFetcher, FTerrainBarrier& TerrainBarrier,
		int32 TileSideVertices, int32 TileOverlapVerties, );
	FTerrainPagerRef* GetNative();
	const FTerrainPagerRef* GetNative() const;
	void ReleaseNative();

	bool AddShovel(FShovelBarrier& Shovel);

	FParticleData GetParticleData() const;

	/**
	 * Writes modified heights to OutHeights and returns an array of modified vertices, for easy
	 * iteration. The BoundVerts parameters is used to describe the vertex count of a grid that this
	 * native is assumed to be placed at its center, and that the vertex indices of this native will
	 * be mapped to.
	 */
	TArray<std::tuple<int32, int32>> GetModifiedHeights(
		TArray<float>& OutHeights, int32 BoundVertsX, int32 BoundVertsY) const;

	FVector GetReferencePoint() const;
	FQuat GetReferenceRotation() const;

private:
	FTerrainPagerBarrier(const FTerrainPagerBarrier&) = delete;
	void operator=(const FTerrainPagerBarrier&) = delete;

	std::unique_ptr<FTerrainPagerRef> NativeRef;
	std::unique_ptr<FTerrainDataSourceRef> DataSourceRef;
};
