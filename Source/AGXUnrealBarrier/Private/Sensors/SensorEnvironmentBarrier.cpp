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
#include <agxSensor/RaytraceAmbientMaterial.h>
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

bool FSensorEnvironmentBarrier::Add(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	agxTerrain::TerrainRef TerrainNative = Terrain.GetNative()->Native;
	const bool Result = NativeRef->Native->add(TerrainNative);
	/*agxSensor::RtSurfaceMaterial::getOrCreate(TerrainNative.get())
		.setReflectivity(Reflectivity);*/ // TODO!!!
	return Result;
}

bool FSensorEnvironmentBarrier::Add(FTerrainPagerBarrier& Pager)
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

void FSensorEnvironmentBarrier::SetAmbientMaterial(FRtAmbientMaterialBarrier* Material)
{
	check(HasNative());
	if (Material == nullptr)
		NativeRef->Native->getScene()->setMaterial({nullptr});
	else
		NativeRef->Native->getScene()->setMaterial(Material->GetNative()->Native);
}

namespace SensorEnvironmentBarrier_helpers
{
	template <typename TBarrier>
	void SetLidarSurfaceMaterialOrDefault(
		TBarrier& Barrier, FRtLambertianOpaqueMaterialBarrier* Material)
	{
		check(Barrier.HasNative());

		if (Material == nullptr)
		{
			// Assign default if setting nullptr Material.
			agxSensor::RtSurfaceMaterial::set(
				Barrier.GetNative()->Native, agxSensor::RtLambertianOpaqueMaterial::create());
		}
		else
		{
			check(Material->HasNative());
			agxSensor::RtSurfaceMaterial::set(
				Barrier.GetNative()->Native, Material->GetNative()->Native);
		}
	}
}

void FSensorEnvironmentBarrier::SetLidarSurfaceMaterialOrDefault(
	FTerrainBarrier& Terrain, FRtLambertianOpaqueMaterialBarrier* Material)
{
	SensorEnvironmentBarrier_helpers::SetLidarSurfaceMaterialOrDefault(Terrain, Material);
}

void FSensorEnvironmentBarrier::SetLidarSurfaceMaterialOrDefault(
	FTerrainPagerBarrier& TerrainPager, FRtLambertianOpaqueMaterialBarrier* Material)
{
	SensorEnvironmentBarrier_helpers::SetLidarSurfaceMaterialOrDefault(TerrainPager, Material);
}
