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
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* GetFrom(const FSimulationBarrier* Barrier);
	static agxCollide::Geometry* GetGeometryFrom(const FShapeBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(const FShapeBarrier* Barrier);
};
