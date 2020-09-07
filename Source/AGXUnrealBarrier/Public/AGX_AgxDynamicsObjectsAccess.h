#pragma once

namespace agx
{
	class RigidBody;
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

class AGXUNREALBARRIER_API FAGX_AgxDynamicsObjectsAccess
{
public:
	static agx::RigidBody* GetFrom(FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* GetFrom(FSimulationBarrier* Barrier);
	static agxCollide::Geometry* GetGeometryFrom(FShapeBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(FShapeBarrier* Barrier);
};
