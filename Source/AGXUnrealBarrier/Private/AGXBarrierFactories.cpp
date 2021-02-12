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
#include <agxModel/TwoBodyTire.h>
#include <EndAGXIncludes.h>

#include <memory>

FRigidBodyBarrier AGXBarrierFactories::CreateRigidBodyBarrier(agx::RigidBody* Body)
{
	return {std::make_unique<FRigidBodyRef>(Body)};
}

FSphereShapeBarrier AGXBarrierFactories::CreateSphereShapeBarrier(agxCollide::Sphere* Sphere)
{
	return {std::make_unique<FGeometryAndShapeRef>(Sphere->getGeometry(), Sphere)};
}

FBoxShapeBarrier AGXBarrierFactories::CreateBoxShapeBarrier(agxCollide::Box* Box)
{
	return {std::make_unique<FGeometryAndShapeRef>(Box->getGeometry(), Box)};
}

FCylinderShapeBarrier AGXBarrierFactories::CreateCylinderShapeBarrier(
	agxCollide::Cylinder* Cylinder)
{
	return {std::make_unique<FGeometryAndShapeRef>(Cylinder->getGeometry(), Cylinder)};
}

FCapsuleShapeBarrier AGXBarrierFactories::CreateCapsuleShapeBarrier(
	agxCollide::Capsule* Capsule)
{
	return {std::make_unique<FGeometryAndShapeRef>(Capsule->getGeometry(), Capsule)};
}

FTrimeshShapeBarrier AGXBarrierFactories::CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh)
{
	return {std::make_unique<FGeometryAndShapeRef>(Trimesh->getGeometry(), Trimesh)};
}

FHingeBarrier AGXBarrierFactories::CreateHingeBarrier(agx::Hinge* Hinge)
{
	return {std::make_unique<FConstraintRef>(Hinge)};
}

FPrismaticBarrier AGXBarrierFactories::CreatePrismaticBarrier(agx::Prismatic* Prismatic)
{
	return {std::make_unique<FConstraintRef>(Prismatic)};
}

FBallJointBarrier AGXBarrierFactories::CreateBallJointBarrier(agx::BallJoint* BallJoint)
{
	return {std::make_unique<FConstraintRef>(BallJoint)};
}

FCylindricalJointBarrier AGXBarrierFactories::CreateCylindricalJointBarrier(
	agx::CylindricalJoint* CylindricalJoint)
{
	return {std::make_unique<FConstraintRef>(CylindricalJoint)};
}

FDistanceJointBarrier AGXBarrierFactories::CreateDistanceJointBarrier(
	agx::DistanceJoint* DistanceJoint)
{
	return {std::make_unique<FConstraintRef>(DistanceJoint)};
}

FLockJointBarrier AGXBarrierFactories::CreateLockJointBarrier(agx::LockJoint* LockJoint)
{
	return {std::make_unique<FConstraintRef>(LockJoint)};
}

FShapeMaterialBarrier AGXBarrierFactories::CreateShapeMaterialBarrier(agx::Material* Material)
{
	return {std::make_unique<FMaterialRef>(Material)};
}

FContactMaterialBarrier AGXBarrierFactories::CreateContactMaterialBarrier(
	agx::ContactMaterial* ContactMaterial)
{
	return {std::make_unique<FContactMaterialRef>(ContactMaterial)};
}

FTwoBodyTireBarrier AGXBarrierFactories::CreateTwoBodyTireBarrier(agxModel::TwoBodyTire* Tire)
{
	return {std::make_unique<FTireRef>(Tire)};
}
