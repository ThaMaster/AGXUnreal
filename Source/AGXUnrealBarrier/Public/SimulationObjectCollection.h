#pragma once

// AGX Dynamics for Unreal includes.
// For some reason, these Shapes could not be forward declared without compiler error.
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/CylinderShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
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

// Others.
class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeMaterialBarrier;
class FTwoBodyTireBarrier;

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

	TArray<FTwoBodyTireBarrier>& GetTwoBodyTires();
	const TArray<FTwoBodyTireBarrier>& GetTwoBodyTires() const;

	TArray<FWireBarrier>& GetWires();
	const TArray<FWireBarrier>& GetWires() const;

private:
	FSimulationObjectCollection(const FSimulationObjectCollection&) = delete;
	void operator=(const FSimulationObjectCollection&) = delete;

	// The Simulation from which all other Simulation Objects collected from.
	FSimulationBarrier Simulation;

	// These are "free" Shapes only, i.e. not owned by a RigidBody.
	TArray<FSphereShapeBarrier> SphereShapes;
	TArray<FBoxShapeBarrier> BoxShapes;
	TArray<FCylinderShapeBarrier> CylinderShapes;
	TArray<FCapsuleShapeBarrier> CapsuleShapes;
	TArray<FTrimeshShapeBarrier> TrimeshShapes;

	// These are "free" Constraints only, i.e. not owned by e.g. Two Body Tire or similar.
	TArray<FHingeBarrier> HingeConstraints;
	TArray<FPrismaticBarrier> PrismaticConstraints;
	TArray<FBallJointBarrier> BallConstraints;
	TArray<FCylindricalJointBarrier> CylindricalConstraints;
	TArray<FDistanceJointBarrier> DistanceConstraints;
	TArray<FLockJointBarrier> LockConstraints;

	TArray<FRigidBodyBarrier> RigidBodies;
	TArray<FContactMaterialBarrier> ContactMaterials;
	TArray<std::pair<FString, FString>> DisabledCollisionGroups;
	TArray<ObserverFrameData> ObserverFrames;
	TArray<FShapeMaterialBarrier> ShapeMaterials;
	TArray<FTwoBodyTireBarrier> TwoBodyTires;
	TArray<FWireBarrier> Wires;
};