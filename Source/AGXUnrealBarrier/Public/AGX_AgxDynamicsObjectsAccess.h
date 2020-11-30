#pragma once

namespace agx
{
	class RigidBody;
	class Hinge;
}

namespace agxSDK
{
	class Simulation;
}

namespace agxCollide
{
	class Geometry;
	class Shape;
}

class FRigidBodyBarrier;
class FSimulationBarrier;
class FShapeBarrier;
class FHingeBarrier;

class AGXUNREALBARRIER_API FAGX_AgxDynamicsObjectsAccess
{
public:
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* GetFrom(const FSimulationBarrier* Barrier);
	static agxCollide::Geometry* GetGeometryFrom(const FShapeBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(const FShapeBarrier* Barrier);
	static agx::Hinge* GetFrom(const FHingeBarrier* Barrier);
};
