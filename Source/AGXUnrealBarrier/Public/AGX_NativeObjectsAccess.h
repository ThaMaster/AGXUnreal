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

class AGXUNREALBARRIER_API FAGX_NativeObjectsAccess
{
public:
	static agx::RigidBody* BarrierToNative(FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* BarrierToNative(FSimulationBarrier* Barrier);
	static agxCollide::Geometry* BarrierToNativeGeometry(FShapeBarrier* Barrier);
	static agxCollide::Shape* BarrierToNativeShape(FShapeBarrier* Barrier);
};
