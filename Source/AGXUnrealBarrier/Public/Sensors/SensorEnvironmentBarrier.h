// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>


class FLidarBarrier;
class FRtAmbientMaterialBarrier;
class FRtLambertianOpaqueMaterialBarrier;
class FSimulationBarrier;
class FTerrainBarrier;
class FTerrainPagerBarrier;

struct FSensorEnvironmentRef;

class AGXUNREALBARRIER_API FSensorEnvironmentBarrier
{
public:
	FSensorEnvironmentBarrier();
	FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native);
	FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other);
	~FSensorEnvironmentBarrier();

	bool HasNative() const;
	void AllocateNative(FSimulationBarrier& Simulation);
	FSensorEnvironmentRef* GetNative();
	const FSensorEnvironmentRef* GetNative() const;
	void ReleaseNative();

	bool Add(FLidarBarrier& Lidar);
	bool Add(FTerrainBarrier& Terrain);
	bool Add(FTerrainPagerBarrier& Pager);

	bool Remove(FTerrainBarrier& Terrain);
	bool Remove(FTerrainPagerBarrier& Pager);

	void SetAmbientMaterial(FRtAmbientMaterialBarrier* Material);

	void SetLidarSurfaceMaterialOrDefault(
		FTerrainBarrier& Terrain, FRtLambertianOpaqueMaterialBarrier* Material);
	void SetLidarSurfaceMaterialOrDefault(
		FTerrainPagerBarrier& TerrainPager, FRtLambertianOpaqueMaterialBarrier* Material);

private:
	FSensorEnvironmentBarrier(const FSensorEnvironmentBarrier&) = delete;
	void operator=(const FSensorEnvironmentBarrier&) = delete;

private:
	std::unique_ptr<FSensorEnvironmentRef> NativeRef;
};
