// Copyright 2023, Algoryx Simulation AB.

#include "SimulationObjectCollection.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "AGXBarrierFactories.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Vehicle/TrackBarrier.h"

FSimulationObjectCollection::~FSimulationObjectCollection()
{
}

TArray<FRigidBodyBarrier>& FSimulationObjectCollection::GetRigidBodies()
{
	return RigidBodies;
}

const TArray<FRigidBodyBarrier>& FSimulationObjectCollection::GetRigidBodies() const
{
	return RigidBodies;
}

TArray<FAnyShapeBarrier> FSimulationObjectCollection::CollectAllShapes() const
{
	TArray<FAnyShapeBarrier> AllShapes;
	auto AddShapes = [&AllShapes](const auto& Shapes)
	{
		for (auto& Shape : Shapes)
		{
			agxCollide::Shape* ShapeAGX = FAGX_AgxDynamicsObjectsAccess::GetShapeFrom(&Shape);
			AllShapes.Add(AGXBarrierFactories::CreateAnyShapeBarrier(ShapeAGX));
		}
	};
	AddShapes(SphereShapes);
	AddShapes(BoxShapes);
	AddShapes(CylinderShapes);
	AddShapes(CapsuleShapes);
	AddShapes(TrimeshShapes);
	for (const FRigidBodyBarrier& Body : RigidBodies)
	{
		AddShapes(Body.GetSphereShapes());
		AddShapes(Body.GetBoxShapes());
		AddShapes(Body.GetCylinderShapes());
		AddShapes(Body.GetCapsuleShapes());
		AddShapes(Body.GetTrimeshShapes());
	}
}

TArray<FSphereShapeBarrier>& FSimulationObjectCollection::GetSphereShapes()
{
	return SphereShapes;
}

const TArray<FSphereShapeBarrier>& FSimulationObjectCollection::GetSphereShapes() const
{
	return SphereShapes;
}

TArray<FBoxShapeBarrier>& FSimulationObjectCollection::GetBoxShapes()
{
	return BoxShapes;
}

const TArray<FBoxShapeBarrier>& FSimulationObjectCollection::GetBoxShapes() const
{
	return BoxShapes;
}

TArray<FCylinderShapeBarrier>& FSimulationObjectCollection::GetCylinderShapes()
{
	return CylinderShapes;
}

const TArray<FCylinderShapeBarrier>& FSimulationObjectCollection::GetCylinderShapes() const
{
	return CylinderShapes;
}

TArray<FCapsuleShapeBarrier>& FSimulationObjectCollection::GetCapsuleShapes()
{
	return CapsuleShapes;
}

const TArray<FCapsuleShapeBarrier>& FSimulationObjectCollection::GetCapsuleShapes() const
{
	return CapsuleShapes;
}

TArray<FTrimeshShapeBarrier>& FSimulationObjectCollection::GetTrimeshShapes()
{
	return TrimeshShapes;
}

const TArray<FTrimeshShapeBarrier>& FSimulationObjectCollection::GetTrimeshShapes() const
{
	return TrimeshShapes;
}

TArray<FHingeBarrier>& FSimulationObjectCollection::GetHingeConstraints()
{
	return HingeConstraints;
}

const TArray<FHingeBarrier>& FSimulationObjectCollection::GetHingeConstraints() const
{
	return HingeConstraints;
}

TArray<FPrismaticBarrier>& FSimulationObjectCollection::GetPrismaticConstraints()
{
	return PrismaticConstraints;
}

const TArray<FPrismaticBarrier>& FSimulationObjectCollection::GetPrismaticConstraints() const
{
	return PrismaticConstraints;
}

TArray<FBallJointBarrier>& FSimulationObjectCollection::GetBallConstraints()
{
	return BallConstraints;
}

const TArray<FBallJointBarrier>& FSimulationObjectCollection::GetBallConstraints() const
{
	return BallConstraints;
}

TArray<FCylindricalJointBarrier>& FSimulationObjectCollection::GetCylindricalConstraints()
{
	return CylindricalConstraints;
}

const TArray<FCylindricalJointBarrier>& FSimulationObjectCollection::GetCylindricalConstraints()
	const
{
	return CylindricalConstraints;
}

TArray<FDistanceJointBarrier>& FSimulationObjectCollection::GetDistanceConstraints()
{
	return DistanceConstraints;
}

const TArray<FDistanceJointBarrier>& FSimulationObjectCollection::GetDistanceConstraints() const
{
	return DistanceConstraints;
}

TArray<FLockJointBarrier>& FSimulationObjectCollection::GetLockConstraints()
{
	return LockConstraints;
}

const TArray<FLockJointBarrier>& FSimulationObjectCollection::GetLockConstraints() const
{
	return LockConstraints;
}

TArray<FContactMaterialBarrier>& FSimulationObjectCollection::GetContactMaterials()
{
	return ContactMaterials;
}

const TArray<FContactMaterialBarrier>& FSimulationObjectCollection::GetContactMaterials() const
{
	return ContactMaterials;
}

TArray<std::pair<FString, FString>>& FSimulationObjectCollection::GetDisabledCollisionGroups()
{
	return DisabledCollisionGroups;
}

const TArray<std::pair<FString, FString>>& FSimulationObjectCollection::GetDisabledCollisionGroups()
	const
{
	return DisabledCollisionGroups;
}

TArray<FSimulationObjectCollection::ObserverFrameData>&
FSimulationObjectCollection::GetObserverFrames()
{
	return ObserverFrames;
}
const TArray<FSimulationObjectCollection::ObserverFrameData>&
FSimulationObjectCollection::GetObserverFrames() const
{
	return ObserverFrames;
}

TArray<FShapeMaterialBarrier>& FSimulationObjectCollection::GetShapeMaterials()
{
	return ShapeMaterials;
}

const TArray<FShapeMaterialBarrier>& FSimulationObjectCollection::GetShapeMaterials() const
{
	return ShapeMaterials;
}

TArray<FTwoBodyTireBarrier>& FSimulationObjectCollection::GetTwoBodyTires()
{
	return TwoBodyTires;
}

const TArray<FTwoBodyTireBarrier>& FSimulationObjectCollection::GetTwoBodyTires() const
{
	return TwoBodyTires;
}

TArray<FWireBarrier>& FSimulationObjectCollection::GetWires()
{
	return Wires;
}

const TArray<FWireBarrier>& FSimulationObjectCollection::GetWires() const
{
	return Wires;
}

TArray<FTrackBarrier>& FSimulationObjectCollection::GetTracks()
{
	return Tracks;
}

const TArray<FTrackBarrier>& FSimulationObjectCollection::GetTracks() const
{
	return Tracks;
}
