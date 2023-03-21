#include "Utilities/TerrainUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Terrain/TerrainBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Physics/GranularBodySystem.h>
#include "EndAGXIncludes.h"

namespace TerrainUtilities_helpers
{
	void AppendParticlePositions(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<FVector>& OutPositions)
	{
		if (GranularParticles.size() > OutPositions.GetSlack())
		{
			OutPositions.Reserve(OutPositions.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Vec3 PositionAGX = GranularParticles[i].position();
			const FVector Position = ConvertDisplacement(PositionAGX);
			OutPositions.Add(Position);
		}
	}

	void AppendParticleRadii(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<float>& OutRadii)
	{
		if (GranularParticles.size() > OutRadii.GetSlack())
		{
			OutRadii.Reserve(OutRadii.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Real RadiusAGX = GranularParticles[i].radius();
			const float Radius = ConvertDistanceToUnreal<float>(RadiusAGX);
			OutRadii.Add(Radius);
		}
	}

	void AppendParticleRotations(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<FQuat>& OutRotations)
	{
		if (GranularParticles.size() > OutRotations.GetSlack())
		{
			OutRotations.Reserve(OutRotations.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Quat RotationAGX = GranularParticles[i].rotation();
			const FQuat Rotation = Convert(RotationAGX);
			OutRotations.Add(Rotation);
		}
	}

	const agx::Physics::GranularBodyPtrArray GetGranularParticles(
		const agxTerrain::Terrain& Terrain)
	{
		return Terrain.getSoilSimulationInterface()->getGranularBodySystem()->getParticles();
	}
}

void FTerrainUtilities::AppendParticlePositions(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	TerrainUtilities_helpers::AppendParticlePositions(GranularParticles, OutPositions);
}

void FTerrainUtilities::AppendParticleRadii(const FTerrainBarrier& Terrain, TArray<float>& OutRadii)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	TerrainUtilities_helpers::AppendParticleRadii(GranularParticles, OutRadii);
}

void FTerrainUtilities::AppendParticleRotations(
	const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	TerrainUtilities_helpers::AppendParticleRotations(GranularParticles, OutRotations);
}

void FTerrainUtilities::AppendParticleData(
	const FTerrainBarrier& Terrain, FParticleData& OutParticleData)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	TerrainUtilities_helpers::AppendParticlePositions(GranularParticles, OutParticleData.Positions);
	TerrainUtilities_helpers::AppendParticleRadii(GranularParticles, OutParticleData.Radii);
	TerrainUtilities_helpers::AppendParticleRotations(GranularParticles, OutParticleData.Rotations);
}

size_t FTerrainUtilities::GetNumParticles(const FTerrainBarrier& Terrain)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return 0;

	return Terrain.GetNative()
		->Native->getSoilSimulationInterface()
		->getGranularBodySystem()
		->getNumParticles();
}
