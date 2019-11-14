#pragma once

#include <memory>

struct FSimulationRef;

class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FMaterialBarrier;
class FTerrainBarrier;

class AGXUNREALBARRIER_API FSimulationBarrier
{
public:
	FSimulationBarrier();
	~FSimulationBarrier();

	void AddRigidBody(FRigidBodyBarrier* Body);
	void AddConstraint(FConstraintBarrier* Constraint);

	void AddMaterial(FMaterialBarrier* Material);
	void RemoveMaterial(FMaterialBarrier* Material);

	void AddContactMaterial(FContactMaterialBarrier* ContactMaterial);
	void RemoveContactMaterial(FContactMaterialBarrier* ContactMaterial);

	void AddTerrain(FTerrainBarrier* Terrain);

	void Step();

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
