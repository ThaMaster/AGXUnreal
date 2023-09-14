#include "Utilities/TerrainUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGXRefs.h"
#include "Terrain/TerrainBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/Terrain.h>
#include <agx/Physics/GranularBodySystem.h>
#include "EndAGXIncludes.h"

namespace TerrainUtilities_helpers
{
	template <typename FUnrealT, typename FGetFunc, typename FConvertFunc>
	void GetParticleDataById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FUnrealT>& Out,
		agxData::IndexArray& IdToIndex, FUnrealT InvalidMarker, FGetFunc GetFunc,
		FConvertFunc ConvertFunc)
	{
		verify(IdToIndex.size() < std::numeric_limits<int32>::max());
		const int32 NumIds = static_cast<int32>(IdToIndex.size());
		Out.SetNum(NumIds);

		for (size_t Id = 0; Id < NumIds; ++Id)
		{
			const size_t Index = IdToIndex[Id];

			// This assumes that the IdToIndex array contains InvalidIndex whenever there isn't an
			// entity with that ID in the storage. Is that always guaranteed?
			if (Index != agx::InvalidIndex)
			{
				const auto& ValueAgx = GetFunc(Particles[Index]);
				const FUnrealT Value = ConvertFunc(ValueAgx);
				Out[Id] = Value;
			}
			else
			{
				// Some types may do validity checks, such as FVector not allowing NaN, in the
				// assignment operator. If this becomes a problem then do memcpy here instead.
				Out[Id] = InvalidMarker;
			}
		}
	}

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

	void GetParticleExistsById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<bool>& OutExists, agxData::IndexArray& IdToIndex)
	{
		verify(IdToIndex.size() < std::numeric_limits<int32>::max());
		const int32 NumIds = static_cast<int32>(IdToIndex.size());
		OutExists.SetNum(NumIds);

		for (size_t Id = 0; Id < NumIds; ++Id)
		{
			const size_t Index = IdToIndex[Id];

			// This assumes that the IdToIndex array contains InvalidIndex whenever there isn't an
			// entity with that ID in the storage. Is that always guaranteed?
			OutExists[Id] = Index != agx::InvalidIndex;
		}
	}

	void GetParticlePositionsById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FVector>& OutPositions,
		agxData::IndexArray& IdToIndex)
	{
		// Create an invalid marker filled with NaN. Cannot use a constructor to initialize it
		// because the constructors check for NaN.
		FVector InvalidMarker;
		constexpr FVector::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = NaN;
		GetParticleDataById(
			Particles, OutPositions, IdToIndex, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.position(); },
			[](const agx::Vec3& ValueAgx) { return ConvertDisplacement(ValueAgx); });
	}

	void AppendParticleVelocities(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FVector>& OutVelocities)
	{
		if (Particles.size() > OutVelocities.GetSlack())
		{
			OutVelocities.Reserve(OutVelocities.Num() + Particles.size());
		}

		for (size_t I = 0; I < Particles.size(); ++I)
		{
			const agx::Vec3 VelocityAgx = Particles[I].velocity();
			const FVector Velocity = ConvertDisplacement(VelocityAgx);
			OutVelocities.Add(Velocity);
		}
	}

	void GetParticleVelocitiesById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FVector>& OutVelocities,
		agxData::IndexArray& IdToIndex)
	{
		// Create an invalid marker filled with NaN. Cannot use a constructor to initialize it
		// because the constructors check for NaN.
		FVector InvalidMarker;
		constexpr FVector::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = NaN;
		GetParticleDataById(
			Particles, OutVelocities, IdToIndex, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.velocity(); },
			[](const agx::Vec3& ValueAgx) { return ConvertDisplacement(ValueAgx); });
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

	void GetParticleRadiiById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<float>& OutRadii,
		agxData::IndexArray& IdToIndex)
	{
		constexpr float InvalidMarker = std::numeric_limits<float>::quiet_NaN();
		GetParticleDataById(
			Particles, OutRadii, IdToIndex, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.radius(); },
			[](float ValueAgx) { return ConvertDistanceToUnreal<float>(ValueAgx); });
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

	void GetParticleRotationsById(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FQuat>& OutRotations,
		agxData::IndexArray& IdToIndex)
	{
		FQuat InvalidMarker;
		constexpr FQuat::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = InvalidMarker.W = NaN;
		GetParticleDataById(
			Particles, OutRotations, IdToIndex, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.rotation(); },
			[](const agx::Quat& ValueAgx) { return Convert(ValueAgx); });
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

void FTerrainUtilities::AppendParticleVelocities(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
	{
		return;
	}

	const agx::Physics::GranularBodyPtrArray Particles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);
	TerrainUtilities_helpers::AppendParticleVelocities(Particles, OutVelocities);
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
	TerrainUtilities_helpers::AppendParticleVelocities(
		GranularParticles, OutParticleData.Velocities);
	TerrainUtilities_helpers::AppendParticleRadii(GranularParticles, OutParticleData.Radii);
	TerrainUtilities_helpers::AppendParticleRotations(GranularParticles, OutParticleData.Rotations);
}

void FTerrainUtilities::GetParticleExistsById(const FTerrainBarrier& Terrain, TArray<bool>& OutExists)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
	{
		return;
	}

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	agxData::EntityStorage* Storage = Terrain.GetNative()
										  ->Native->getSoilSimulationInterface()
										  ->getGranularBodySystem()
										  ->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();

	TerrainUtilities_helpers::GetParticleExistsById(GranularParticles, OutExists, IdToIndex);
}

void FTerrainUtilities::GetParticlePositionsById(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	const agx::Physics::GranularBodyPtrArray GranularParticles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	agxData::EntityStorage* Storage = Terrain.GetNative()
										  ->Native->getSoilSimulationInterface()
										  ->getGranularBodySystem()
										  ->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();

	TerrainUtilities_helpers::GetParticlePositionsById(GranularParticles, OutPositions, IdToIndex);
}

void FTerrainUtilities::GetParticleVelocitiesById(const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities)
{
	AGX_CHECK(Terrain.HasNative())
	if (!Terrain.HasNative())
	{
		return;
	}

	const agx::Physics::GranularBodyPtrArray Particles = TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	agxData::EntityStorage* Storage = Terrain.GetNative()->Native->getSoilSimulationInterface()->getGranularBodySystem()->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();
	TerrainUtilities_helpers::GetParticleVelocitiesById(Particles, OutVelocities, IdToIndex);
}



void FTerrainUtilities::GetParticleRotationsById(const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations)
{
	AGX_CHECK(Terrain.HasNative())
	if (!Terrain.HasNative())
	{
		return;
	}

	const agx::Physics::GranularBodyPtrArray Particles = TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	agxData::EntityStorage* Storage = Terrain.GetNative()->Native->getSoilSimulationInterface()->getGranularBodySystem()->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();
	TerrainUtilities_helpers::GetParticleRotationsById(Particles, OutRotations, IdToIndex);
}

void FTerrainUtilities::GetParticleRadiiById(const FTerrainBarrier& Terrain, TArray<float>& OutRadii)
{
	AGX_CHECK(Terrain.HasNative())
		if (!Terrain.HasNative())
		{
			return;
		}

	const agx::Physics::GranularBodyPtrArray Particles = TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	agxData::EntityStorage* Storage = Terrain.GetNative()->Native->getSoilSimulationInterface()->getGranularBodySystem()->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();
	TerrainUtilities_helpers::GetParticleRadiiById(Particles, OutRadii, IdToIndex);
}

void FTerrainUtilities::GetParticleDataById(
	const FTerrainBarrier& Terrain, FParticleDataById& OutParticleData)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	agxData::EntityStorage* Storage = Terrain.GetNative()
										  ->Native->getSoilSimulationInterface()
										  ->getGranularBodySystem()
										  ->getParticleStorage();

	agxData::IndexArray& IdToIndex = Storage->getIdToIndexTable();

	const agx::Physics::GranularBodyPtrArray Particles =
		TerrainUtilities_helpers::GetGranularParticles(*Terrain.GetNative()->Native);

	TerrainUtilities_helpers::GetParticlePositionsById(
		Particles, OutParticleData.Positions, IdToIndex);
	TerrainUtilities_helpers::GetParticleVelocitiesById(
		Particles, OutParticleData.Velocities, IdToIndex);
	TerrainUtilities_helpers::GetParticleRadiiById(Particles, OutParticleData.Radii, IdToIndex);
	TerrainUtilities_helpers::GetParticleRotationsById(
		Particles, OutParticleData.Rotations, IdToIndex);
}

size_t FTerrainUtilities::GetNumParticles(const FTerrainBarrier& Terrain)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return 0;

	return Terrain.GetNative()->Native->getSoilSimulationInterface()->getNumSoilParticles();
}
