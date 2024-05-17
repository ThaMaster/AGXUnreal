// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/SensorEnvironmentBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarBarrier.h"
#include "Sensors/SensorRef.h"
#include "SimulationBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceSurfaceMaterial.h>
#include "EndAGXIncludes.h"

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier()
	: NativeRef {new FSensorEnvironmentRef}
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native)
	: NativeRef(std::move(Native))
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FSensorEnvironmentRef);
}

FSensorEnvironmentBarrier::~FSensorEnvironmentBarrier()
{
}

bool FSensorEnvironmentBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FSensorEnvironmentBarrier::AllocateNative(FSimulationBarrier& Simulation)
{
	check(!HasNative());
	check(Simulation.HasNative());
	NativeRef->Native = agxSensor::Environment::getOrCreate(Simulation.GetNative()->Native);
}

FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FSensorEnvironmentBarrier::ReleaseNative()
{
}

bool FSensorEnvironmentBarrier::Add(FLidarBarrier& Lidar)
{
	check(HasNative());
	check(Lidar.HasNative());
	return NativeRef->Native->add(Lidar.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Add(FTerrainBarrier& Terrain, float Reflectivity)
{
	check(HasNative());
	check(Terrain.HasNative());
	agxTerrain::TerrainRef TerrainNative = Terrain.GetNative()->Native;
	const bool Result = NativeRef->Native->add(TerrainNative);
	agxSensor::RtSurfaceMaterial::getOrCreate(TerrainNative.get())
		.setReflectivity(Reflectivity);
	return Result;
}

bool FSensorEnvironmentBarrier::Add(FTerrainPagerBarrier& Pager, float Reflectivity)
{
	check(HasNative());
	check(Pager.HasNative());
	const bool Result = NativeRef->Native->add(Pager.GetNative()->Native);
#if 0 // TODO: Enable once supported in AGX.
	agxSensor::RtSurfaceMaterial::getOrCreate(Pager.GetNative()->Native.get())
		.setReflectivity(Reflectivity);
#endif
	return Result;
}

bool FSensorEnvironmentBarrier::Remove(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	return NativeRef->Native->remove(Terrain.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Remove(FTerrainPagerBarrier& Pager)
{
	check(HasNative());
	check(Pager.HasNative());
	return NativeRef->Native->remove(Pager.GetNative()->Native);
}
