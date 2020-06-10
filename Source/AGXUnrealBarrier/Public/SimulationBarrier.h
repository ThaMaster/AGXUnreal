#pragma once

#include "Containers/UnrealString.h"

#include <memory>

struct FSimulationRef;

class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeBarrier;
class FShapeMaterialBarrier;
class FTerrainBarrier;

class AGXUNREALBARRIER_API FSimulationBarrier
{
public:
	FSimulationBarrier();
	~FSimulationBarrier();

	void AddRigidBody(FRigidBodyBarrier* Body);
	void AddShape(FShapeBarrier* Shape);
	void AddConstraint(FConstraintBarrier* Constraint);

	void AddShapeMaterial(FShapeMaterialBarrier* Material);
	void RemoveShapeMaterial(FShapeMaterialBarrier* Material);

	void AddContactMaterial(FContactMaterialBarrier* ContactMaterial);
	void RemoveContactMaterial(FContactMaterialBarrier* ContactMaterial);

	void AddTerrain(FTerrainBarrier* Terrain);

	void SetDisableCollisionGroupPair(const FName& Group1, const FName& Group2);

	bool WriteAGXArchive(const FString& Filename) const;

	void EnableRemoteDebugging(int16 Port);

	void Step();

	void EnableStatistics();
	float GetStatistics();

	bool HasNative() const;
	void AllocateNative();
	FSimulationRef* GetNative();
	const FSimulationRef* GetNative() const;
	void ReleaseNative();

private:
	FSimulationBarrier(const FSimulationBarrier&) = delete;
	void operator=(const FSimulationBarrier&) = delete;

private:
	std::unique_ptr<FSimulationRef> NativeRef;
};
