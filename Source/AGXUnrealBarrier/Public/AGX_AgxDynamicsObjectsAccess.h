#pragma once

namespace agx
{
	class RigidBody;

	class BallJoint;
	class CylindricalJoint;
	class DistanceJoint;
	class Hinge;
	class LockJoint;
	class Prismatic;
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

class FBallJointBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FHingeBarrier;
class FLockJointBarrier;
class FPrismaticBarrier;


class AGXUNREALBARRIER_API FAGX_AgxDynamicsObjectsAccess
{
public:
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier* Barrier);
	static agxSDK::Simulation* GetFrom(const FSimulationBarrier* Barrier);

	static agxCollide::Geometry* GetGeometryFrom(const FShapeBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(const FShapeBarrier* Barrier);

	static agx::BallJoint* GetFrom(const FBallJointBarrier* Barrier);
	static agx::CylindricalJoint* GetFrom(const FCylindricalJointBarrier* Barrier);
	static agx::DistanceJoint* GetFrom(const FDistanceJointBarrier* Barrier);
	static agx::Hinge* GetFrom(const FHingeBarrier* Barrier);
	static agx::LockJoint* GetFrom(const FLockJointBarrier* Barrier);
	static agx::Prismatic* GetFrom(const FPrismaticBarrier* Barrier);
};
