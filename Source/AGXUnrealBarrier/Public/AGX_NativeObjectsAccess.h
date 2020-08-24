#pragma once

namespace agx
{
	class RigidBody;
}

namespace agxSDK
{
	class Simulation;
}

class FRigidBodyBarrier;
class FSimulationBarrier;

class AGXUNREALBARRIER_API FAGX_NativeObjectsAccess
{
public:
	static agx::RigidBody* BarrierToNative(FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* BarrierToNative(FSimulationBarrier* Barrier);
};
