// Copyright 2022, Algoryx Simulation AB.

#pragma once

namespace agx
{
	class RigidBody;
	class MassProperties;

	class Constraint;

	class BallJoint;
	class CylindricalJoint;
	class DistanceJoint;
	class Hinge;
	class LockJoint;
	class Prismatic;

	class Material;
	class ContactMaterial;
}

namespace agxSDK
{
	class MergeSplitProperties;
	class MergeSplitThresholds;
	class Simulation;
}

namespace agxCollide
{
	class Geometry;
	class Shape;
	class RenderData;
	class GeometryContact;
	class ContactPoint;
}

namespace agxModel
{
	class Tire;
	class TwoBodyTire;
};

namespace agxSDK
{
	class Simulation;
}

namespace agxTerrain
{
	class Terrain;
	class TerrainMaterial;
	class Shovel;
}

namespace agxWire
{
	class Wire;
	class WireWinch;
	class Node;
}

namespace agxWire
{
	class Wire;
}

class FRigidBodyBarrier;
class FMassPropertiesBarrier;

class FConstraintBarrier;

class FBallJointBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FHingeBarrier;
class FLockJointBarrier;
class FPrismaticBarrier;
class FShapeMaterialBarrier;

class FShapeMaterialBarrier;
class FContactMaterialBarrier;

class FShapeBarrier;
class FRenderDataBarrier;
class FShapeContactBarrier;
class FContactPointBarrier;

class FTireBarrier;
class FTwoBodyTireBarrier;

class FSimulationBarrier;

class FTerrainBarrier;
class FTerrainMaterialBarrier;
class FShovelBarrier;

class FWireBarrier;
class FWireWinchBarrier;
class FWireNodeBarrier;

class FMergeSplitPropertiesBarrier;
class FMergeSplitThresholdsBarrier;

class FWireBarrier;

class AGXUNREALBARRIER_API FAGX_AgxDynamicsObjectsAccess
{
public:

	// Namespace agx.

	static agx::RigidBody* GetFrom(const FRigidBodyBarrier* Barrier);
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier& Barrier);
	static agx::RigidBody* TryGetFrom(const FRigidBodyBarrier* Barrier);
	static agx::RigidBody* TryGetFrom(const FRigidBodyBarrier& Barrier);

	static agx::MassProperties* GetFrom(const FMassPropertiesBarrier* Barrier);

	static agx::Constraint* GetFrom(const FConstraintBarrier* Barrier);

	static agx::BallJoint* GetFrom(const FBallJointBarrier* Barrier);
	static agx::CylindricalJoint* GetFrom(const FCylindricalJointBarrier* Barrier);
	static agx::DistanceJoint* GetFrom(const FDistanceJointBarrier* Barrier);
	static agx::Hinge* GetFrom(const FHingeBarrier* Barrier);
	static agx::LockJoint* GetFrom(const FLockJointBarrier* Barrier);
	static agx::Prismatic* GetFrom(const FPrismaticBarrier* Barrier);

	static agx::Material* GetFrom(const FShapeMaterialBarrier* Barrier);
	static agx::ContactMaterial* GetFrom(const FContactMaterialBarrier* Barrier);

	// Namespace agxCollide.

	static agxCollide::Geometry* GetGeometryFrom(const FShapeBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(const FShapeBarrier* Barrier);
	static agxCollide::RenderData* GetFrom(const FRenderDataBarrier* Barrier);
	static agxCollide::GeometryContact* GetFrom(const FShapeContactBarrier* Barrier);
	static agxCollide::ContactPoint* GetFrom(const FContactPointBarrier* Barrier);

	// Namespace agxModel.

	static agxModel::Tire* GetFrom(const FTireBarrier* Barrier);
	static agxModel::TwoBodyTire* GetFrom(const FTwoBodyTireBarrier* Barrier);

	// Namespace agxSDK.

	static agxSDK::Simulation* GetFrom(const FSimulationBarrier* Barrier);
	static agxSDK::MergeSplitProperties* GetFrom(const FMergeSplitPropertiesBarrier* Barrier);
	static agxSDK::MergeSplitThresholds* GetFrom(const FMergeSplitThresholdsBarrier* Barrier);

	// Namespace agxTerrain.

	static agxTerrain::Terrain* GetFrom(const FTerrainBarrier* Barrier);
	static agxTerrain::TerrainMaterial* GetFrom(const FTerrainMaterialBarrier* Barrier);
	static agxTerrain::Shovel* GetFrom(const FShovelBarrier* Barrier);

	// Namespace agxWire.

	static agxWire::Wire* GetFrom(const FWireBarrier* Barrier);
	static agxWire::WireWinch* GetFrom(const FWireWinchBarrier* Barrier);
	static agxWire::Node* GetFrom(const FWireNodeBarrier* Barrier);
};
