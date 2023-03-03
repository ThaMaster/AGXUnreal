// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <memory>

class FTerrainDataSource;
class FTerrainHeightFetcherBase;
struct FTerrainPagerRef;

class AGXUNREALBARRIER_API FTerrainPagerBarrier
{
public:
	FTerrainPagerBarrier();
	FTerrainPagerBarrier(std::unique_ptr<FTerrainPagerRef> InNativeRef);
	FTerrainPagerBarrier(FTerrainPagerBarrier&& Other);
	~FTerrainPagerBarrier();

	bool HasNative() const;
	void AllocateNative(FTerrainHeightFetcherBase* HeightFetcher);
	FTerrainPagerRef* GetNative();
	const FTerrainPagerRef* GetNative() const;
	void ReleaseNative();

 private:
	FTerrainPagerBarrier(const FTerrainPagerBarrier&) = delete;
	void operator=(const FTerrainPagerBarrier&) = delete;

	std::unique_ptr<FTerrainPagerRef> NativeRef;
	std::unique_ptr<FTerrainDataSource> DataSource;
};
