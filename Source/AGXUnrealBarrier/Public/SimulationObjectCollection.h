#pragma once

// AGX Dynamics for Unreal includes.
#include "SimulationBarrier.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"

// Constraints.
class FHingeBarrier;
class FPrismaticBarrier;
class FBallJointBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FLockJointBarrier;

// Shapes.
class FSphereShapeBarrier;
class FBoxShapeBarrier;
class FCylinderShapeBarrier;
class FCapsuleShapeBarrier;
class FTrimeshShapeBarrier;

// Others.
class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeMaterialBarrier;
class FTireBarrier;

struct AGXUNREALBARRIER_API FSimulationObjectCollection
{
public:
	FSimulationObjectCollection() = default;
	FSimulationObjectCollection(FSimulationObjectCollection&&) = default;
	FSimulationObjectCollection& operator=(FSimulationObjectCollection&&) = default;
	~FSimulationObjectCollection();

	struct ObserverFrameData
	{
		FString Name;
		FGuid BodyGuid;
		FTransform Transform;
	};

	TArray<FRigidBodyBarrier>& GetRigidBodies();
	const TArray<FRigidBodyBarrier>& GetRigidBodies() const;

	TArray<FSphereShapeBarrier>& GetSphereShapes();
	const TArray<FSphereShapeBarrier>& GetSphereShapes() const;

	TArray<FBoxShapeBarrier>& GetBoxShapes();
	const TArray<FBoxShapeBarrier>& GetBoxShapes() const;

	TArray<FCylinderShapeBarrier>& GetCylinderShapes();
	const TArray<FCylinderShapeBarrier>& GetCylinderShapes() const;

	TArray<FCapsuleShapeBarrier>& GetCapsuleShapes();
	const TArray<FCapsuleShapeBarrier>& GetCapsuleShapes() const;

	TArray<FTrimeshShapeBarrier>& GetTrimeshShapes();
	const TArray<FTrimeshShapeBarrier>& GetTrimeshShapes() const;

	TArray<FHingeBarrier>& GetHingeConstraints();
	const TArray<FHingeBarrier>& GetHingeConstraints() const;

	TArray<FPrismaticBarrier>& GetPrismaticConstraints();
	const TArray<FPrismaticBarrier>& GetPrismaticConstraints() const;

	TArray<FBallJointBarrier>& GetBallConstraints();
	const TArray<FBallJointBarrier>& GetBallConstraints() const;

	TArray<FCylindricalJointBarrier>& GetCylindricalConstraints();
	const TArray<FCylindricalJointBarrier>& GetCylindricalConstraints() const;

	TArray<FDistanceJointBarrier>& GetDistanceConstraints();
	const TArray<FDistanceJointBarrier>& GetDistanceConstraints() const;

	TArray<FLockJointBarrier>& GetLockConstraints();
	const TArray<FLockJointBarrier>& GetLockConstraints() const;

	TArray<FContactMaterialBarrier>& GetContactMaterials();
	const TArray<FContactMaterialBarrier>& GetContactMaterials() const;

	TArray<std::pair<FString, FString>>& GetDisabledCollisionGroups();
	const TArray<std::pair<FString, FString>>& GetDisabledCollisionGroups() const;

	TArray<ObserverFrameData>& GetObserverFrames();
	const TArray<ObserverFrameData>& GetObserverFrames() const;

	TArray<FShapeMaterialBarrier>& GetShapeMaterials();
	const TArray<FShapeMaterialBarrier>& GetShapeMaterials() const;

	TArray<FTireBarrier>& GetTires();
	const TArray<FTireBarrier>& GetTires() const;

	TArray<FWireBarrier>& GetWires();
	const TArray<FWireBarrier>& GetWires() const;

private:
	FSimulationObjectCollection(const FSimulationObjectCollection&) = delete;
	void operator=(const FSimulationObjectCollection&) = delete;

	// The Simulation from which all other Simulation Objects collected from.
	FSimulationBarrier Simulation;

	// These are "free" RigidBodies only, i.e. not owned by PowerLine, Tire, Wire etc.
	TArray<FRigidBodyBarrier> RigidBodies;

	// These are "free" Shapes only, i.e. not owned by a RigidBody.
	TArray<FSphereShapeBarrier> SphereShapes;
	TArray<FBoxShapeBarrier> BoxShapes;
	TArray<FCylinderShapeBarrier> CylinderShapes;
	TArray<FCapsuleShapeBarrier> CapsuleShapes;
	TArray<FTrimeshShapeBarrier> TrimeshShapes;

	TArray<FHingeBarrier> HingeConstraints;
	TArray<FPrismaticBarrier> PrismaticConstraints;
	TArray<FBallJointBarrier> BallConstraints;
	TArray<FCylindricalJointBarrier> CylindricalConstraints;
	TArray<FDistanceJointBarrier> DistanceConstraints;
	TArray<FLockJointBarrier> LockConstraints;


	TArray<FContactMaterialBarrier> ContactMaterials;
	TArray<std::pair<FString, FString>> DisabledCollisionGroups;
	TArray<ObserverFrameData> ObserverFrames;
	TArray<FShapeMaterialBarrier> ShapeMaterials;
	TArray<FTireBarrier> Tires;
	TArray<FWireBarrier> Wires;
};