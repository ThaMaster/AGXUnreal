
#include "SimulationObjectCollection.h"

#include "RigidBodyBarrier.h"
#include "Constraints/ConstraintBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TireBarrier.h"
#include "Wire/WireBarrier.h"

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

TArray<TUniquePtr<FShapeBarrier>>& FSimulationObjectCollection::GetShapes()
{
	return Shapes;
}
const TArray<TUniquePtr<FShapeBarrier>>& FSimulationObjectCollection::GetShapes() const
{
	return Shapes;
}

TArray<TUniquePtr<FConstraintBarrier>>& FSimulationObjectCollection::GetConstraints()
{
	return Constraints;
}
const TArray<TUniquePtr<FConstraintBarrier>>& FSimulationObjectCollection::GetConstraints() const
{
	return Constraints;
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

TArray<FTireBarrier>& FSimulationObjectCollection::GetTires()
{
	return Tires;
}

const TArray<FTireBarrier>& FSimulationObjectCollection::GetTires() const
{
	return Tires;
}

TArray<FWireBarrier>& FSimulationObjectCollection::GetWires()
{
	return Wires;
}

const TArray<FWireBarrier>& FSimulationObjectCollection::GetWires() const
{
	return Wires;
}
