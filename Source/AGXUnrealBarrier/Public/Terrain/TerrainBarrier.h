// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

#include <memory>

struct FTerrainRef;

class FHeightFieldShapeBarrier;
class FShovelBarrier;
class FTerrainMaterialBarrier;
class FShapeMaterialBarrier;

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

	bool AddShovel(FShovelBarrier& Shovel);
	void SetShapeMaterial(const FShapeMaterialBarrier& Material);
	void SetTerrainMaterial(const FTerrainMaterialBarrier& TerrainMaterial);

	int32 GetGridSizeX() const;
	int32 GetGridSizeY() const;

	/**
	 * Get an array with all the heights in the height field, stored in X major
	 * order, meaning that heights with increasing the X coordinates are next to
	 * each other in memory.
	 */
	TArray<float> GetHeights() const;

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

private:
	FTerrainBarrier(const FTerrainBarrier&) = delete;
	void operator=(const FTerrainBarrier&) = delete;

private:
	std::unique_ptr<FTerrainRef> NativeRef;
};
