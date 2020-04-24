#include "AGXBarrierFactories.h"

#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Hinge.h>
#include <agx/Prismatic.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/LockJoint.h>
#include <agxCollide/Sphere.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include <EndAGXIncludes.h>

#include <memory>

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body)
{
	return {std::make_unique<FRigidBodyRef>(Body)};
}

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere)
{
	return {std::make_unique<FGeometryAndShapeRef>(Sphere->getGeometry(), Sphere)};
}

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box)
{
	return {std::make_unique<FGeometryAndShapeRef>(Box->getGeometry(), Box)};
}

FCylinderShapeBarrier CreateCylinderShapeBarrier(agxCollide::Cylinder* Cylinder)
{
	return {std::make_unique<FGeometryAndShapeRef>(Cylinder->getGeometry(), Cylinder)};
}

FTrimeshShapeBarrier CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh)
{
	return {std::make_unique<FGeometryAndShapeRef>(Trimesh->getGeometry(), Trimesh)};
}

FHingeBarrier CreateHingeBarrier(agx::Hinge* Hinge)
{
	return {std::make_unique<FConstraintRef>(Hinge)};
}

FPrismaticBarrier CreatePrismaticBarrier(agx::Prismatic* Prismatic)
{
	return {std::make_unique<FConstraintRef>(Prismatic)};
}

FBallJointBarrier CreateBallJointBarrier(agx::BallJoint* BallJoint)
{
	return {std::make_unique<FConstraintRef>(BallJoint)};
}

FCylindricalJointBarrier CreateCylindricalJointBarrier(agx::CylindricalJoint* CylindricalJoint)
{
	return {std::make_unique<FConstraintRef>(CylindricalJoint)};
}

FDistanceJointBarrier CreateDistanceJointBarrier(agx::DistanceJoint* DistanceJoint)
{
	return {std::make_unique<FConstraintRef>(DistanceJoint)};
}

FLockJointBarrier CreateLockJointBarrier(agx::LockJoint* LockJoint)
{
	return {std::make_unique<FConstraintRef>(LockJoint)};
}
