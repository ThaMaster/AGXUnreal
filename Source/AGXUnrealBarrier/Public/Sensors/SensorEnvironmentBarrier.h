// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>


class FTerrainBarrier;
class FTerrainPagerBarrier;
class FLidarBarrier;

struct FSensorEnvironmentRef;

class AGXUNREALBARRIER_API FSensorEnvironmentBarrier
{
public:
	FSensorEnvironmentBarrier();
	FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native);
	FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other);
	~FSensorEnvironmentBarrier();

	bool HasNative() const;
	void AllocateNative();
	FSensorEnvironmentRef* GetNative();
	const FSensorEnvironmentRef* GetNative() const;
	void ReleaseNative();

	bool Add(FLidarBarrier& Lidar);
	bool Add(FTerrainBarrier& Terrain);
	bool Add(FTerrainPagerBarrier& Pager);

	void Step(double DeltaTime);

private:
	FSensorEnvironmentBarrier(const FSensorEnvironmentBarrier&) = delete;
	void operator=(const FSensorEnvironmentBarrier&) = delete;

private:
	std::unique_ptr<FSensorEnvironmentRef> NativeRef;
};
