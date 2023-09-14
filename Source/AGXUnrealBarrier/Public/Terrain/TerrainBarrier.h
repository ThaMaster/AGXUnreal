// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>
#include <tuple>

struct FTerrainRef;

class FHeightFieldShapeBarrier;
class FShovelBarrier;
class FTerrainMaterialBarrier;
class FShapeMaterialBarrier;

struct FParticleData
{
	TArray<FVector> Positions;
	TArray<FVector> Velocities;
	TArray<float> Radii;
	TArray<FQuat> Rotations;
};

/**
 * Particle data arrays where the per-particle data is stored by-entity-ID instead of packed
 * together. This means that data for a particular particle will always be at the same index in
 * these arrays since a particle always has the same ID. It also means that there may be gaps in
 * the arrays since IDs used by particles removed from the simulation won't be reused until enough
 * new particles has been created to fill the gaps.
 *
 * This data layout is useful when rendering with Niagara because the data reshuffling made by
 * packed data arrays is seen by Niagara as very fast moving particles, which produces rendering
 * artifacts. With this approach particles have a persistent and stable concept of "self". It is
 * possible that there are better ways to solve the problem, maybe with the PARTICLES.ID Niagara
 * attribute, but I don't yet know how.
 */
struct FParticleDataById
{
	TArray<FVector> Positions;
	TArray<FVector> Velocities;
	TArray<float> Radii;
	TArray<FQuat> Rotations;
	TArray<bool> Exists; // TArray instead of TBitArray to be compatible with Niagara Arrays.
};

namespace ParticleDataFlags
{
	enum Type
	{
		Positions = 1 << 0,
		Velocities = 1 << 1,
		Radii = 1 << 2,
		Rotations = 1 << 3,
		All = Positions | Velocities | Radii | Rotations
	};
}

using EParticleDataFlags = ParticleDataFlags::Type;

inline EParticleDataFlags operator|(EParticleDataFlags Lhs, EParticleDataFlags Rhs)
{
	return static_cast<EParticleDataFlags>(
		static_cast<std::underlying_type<EParticleDataFlags>::type>(Lhs) |
		static_cast<std::underlying_type<EParticleDataFlags>::type>(Rhs));
}

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
	void AllocateNative(FHeightFieldShapeBarrier& SourceHeightField, double MaxDepth);
	FTerrainRef* GetNative();
	const FTerrainRef* GetNative() const;
	void ReleaseNative();

	void SetPosition(const FVector& Position);
	FVector GetPosition() const;

	void SetRotation(const FQuat& Rotation);
	FQuat GetRotation() const;

	void SetCreateParticles(bool CreateParticles);
	bool GetCreateParticles() const;

	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);
	bool GetDeleteParticlesOutsideBounds() const;

	void SetPenetrationForceVelocityScaling(double PenetrationForceVelocityScaling);
	double GetPenetrationForceVelocityScaling() const;

	void SetMaximumParticleActivationVolume(double MaximumParticleActivationVolume);
	double GetMaximumParticleActivationVolume() const;

	bool AddShovel(FShovelBarrier& Shovel);
	void SetShapeMaterial(const FShapeMaterialBarrier& Material);
	void SetTerrainMaterial(const FTerrainMaterialBarrier& TerrainMaterial);

	void AddCollisionGroup(const FName& GroupName);
	void AddCollisionGroups(const TArray<FName>& GroupNames);
	TArray<FName> GetCollisionGroups() const;
	void RemoveCollisionGroup(const FName& GroupName);

	/**
	 * Clears both the internal shape and terrain materials.
	 */
	void ClearMaterial();

	int32 GetGridSizeX() const;
	int32 GetGridSizeY() const;

	/**
	 * Returns the modified vertices since the last AGX Dynamics Step Forward.
	 * The x/y layout matches that of an Unreal Landscape coordinate system.
	 *
	 * Example of how to iterate over heights using GetModifiedVertices:
	 * for (const auto& VertexTuple : GetModifiedVertices())
	 * {
	 *		const int32 VertX = std::get<0>(VertexTuple);
	 *		const int32 VertY = std::get<1>(VertexTuple);
	 *		const int32 Index = VertX + VertY * TerrainVerticesX;
	 *		float Height = Heights[Index];
	 *	}
	 */
	TArray<std::tuple<int32, int32>> GetModifiedVertices() const;

	/**
	 * Fill an array with all the heights in the height field, stored in X major
	 * order, meaning that heights with increasing the X coordinates are next to
	 * each other in memory.
	 * If the ChangesOnly parameter is set to true, only height changes from the previous AGX
	 * Dynamics Step Forward is read. This can be used as a performance optimization, but use with
	 * caution; the caller may miss height changes if this is not called each AGX Dynamics Step
	 * Forward. If ChangesOnly is true, then the GetModifiedVertices can be used when iterating over
	 * the heights array at the call site, instead of iterating over all heights, as a further
	 * optimization.
	 */
	void GetHeights(TArray<float>& OutHeights, bool bChangesOnly) const;

	/**
	 * Get an array with the positions of the currently existing particles.
	 */
	TArray<FVector> GetParticlePositions() const;

	/**
	 * Get an array with the radii of the currently existing particles.
	 */
	TArray<float> GetParticleRadii() const;

	/**
	 * Get an array with the rotations of the currently existing particles.
	 */
	TArray<FQuat> GetParticleRotations() const;

	/**
	 * Get Positions, Radii and Rotations of all particles.
	 */
	FParticleData GetParticleData() const;

	/**
	 * Get Positions, Radii, and Rotations of all particles.
	 *
	 * The resulting buffers are populated by entity ID, not by index, which means that there may be
	 * gaps in the data. An Exists array of bools indicate whether or not there is particle data at
	 * a particular index.
	 */
	FParticleDataById GetParticleDataById(EParticleDataFlags ToInclude) const;

	/**
	 * Returns the number of spawned Terrain particles known by the Terrain Native.
	 */
	size_t GetNumParticles() const;

private:
	FTerrainBarrier(const FTerrainBarrier&) = delete;
	void operator=(const FTerrainBarrier&) = delete;

private:
	std::unique_ptr<FTerrainRef> NativeRef;
};
