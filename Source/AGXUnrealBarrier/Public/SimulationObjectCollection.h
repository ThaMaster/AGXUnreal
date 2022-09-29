#pragma once

// AGX Dynamics for Unreal includes.
#include "SimulationBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"

class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeBarrier;
class FShapeMaterialBarrier;
class FTerrainBarrier;
class FTireBarrier;


struct AGXUNREALBARRIER_API FSimulationObjectCollection
{
public:
	FSimulationObjectCollection() = default;
	FSimulationObjectCollection(FSimulationObjectCollection&&) = default;
	FSimulationObjectCollection& operator=(FSimulationObjectCollection&&) = default;
	~FSimulationObjectCollection();

	void SetIsValid(bool InIsValid);
	bool GetIsValid();

	TArray<FRigidBodyBarrier>& GetRigidBodies();
	const TArray<FRigidBodyBarrier>& GetRigidBodies() const;

	TArray<FShapeBarrier>& GetShapes();
	const TArray<FShapeBarrier>& GetShapes() const;

	TArray<FConstraintBarrier>& GetConstraints();
	const TArray<FConstraintBarrier>& GetConstraints() const;

	TArray<FContactMaterialBarrier>& GetContactMaterials();
	const TArray<FContactMaterialBarrier>& GetContactMaterials() const;

	TArray<FShapeMaterialBarrier>& GetShapeMaterials();
	const TArray<FShapeMaterialBarrier>& GetShapeMaterials() const;

	TArray<FTireBarrier>& GetTires();
	const TArray<FTireBarrier>& GetTires() const;

	TArray<FWireBarrier>& GetWires();
	const TArray<FWireBarrier>& GetWires() const;

private:
	FSimulationObjectCollection(const FSimulationObjectCollection&) = delete;
	void operator=(const FSimulationObjectCollection&) = delete;

	// Indicates whether all objects from the Simulation is referenced from this instance.
	bool IsValid = false;

	// The Simulation from which all other Simulation Objects collected from.
	FSimulationBarrier Simulation;

	// These are "free" RigidBodies only, i.e. not owned by PowerLine, Tire, Wire etc.
	TArray<FRigidBodyBarrier> RigidBodies;

	// These are "free" Shapes only, i.e. not owned by a RigidBody.
	TArray<FShapeBarrier> Shapes;

	TArray<FConstraintBarrier> Constraints;
	TArray<FContactMaterialBarrier> ContactMaterials;
	TArray<FShapeMaterialBarrier> ShapeMaterials;
	TArray<FTireBarrier> Tires;
	TArray<FWireBarrier> Wires;
};