#pragma once

#include "CoreMinimal.h"

#include <memory>

struct FTerrainRef;

class FHeightFieldShapeBarrier;
class FShovelBarrier;

/**
 *
 */
class AGXUNREALBARRIER_API FTerrainBarrier
{
public:
	FTerrainBarrier();
	FTerrainBarrier(std::unique_ptr<FTerrainRef> InNativeRef);
	FTerrainBarrier(FTerrainBarrier&& Other);
	~FTerrainBarrier();

	bool HasNative() const;
	void AllocateNative(FHeightFieldShapeBarrier& SourceHeightField);
	FTerrainRef* GetNative();
	const FTerrainRef* GetNative() const;
	void ReleaseNative();

	bool AddShovel(FShovelBarrier& Shovel);

	int32 GetGridSizeX() const;
	int32 GetGridSizeY() const;

	// Get an array with all the heights in the height field, stored in X major
	// order, meaning that heights with increasing the X coordinates are next to
	// each other in memory.
	TArray<float> GetHeights() const;

private:
	FTerrainBarrier(const FTerrainBarrier&) = delete;
	void operator=(const FTerrainBarrier&) = delete;

private:
	std::unique_ptr<FTerrainRef> NativeRef;
};
