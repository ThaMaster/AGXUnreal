#pragma once

#include "RigidBodyBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/CylinderShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"

#include "AGXRefs.h"

namespace agx
{
	class RigidBody;
	class Hinge;
	class Prismatic;
	class BallJoint;
	class CylindricalJoint;
	class DistanceJoint;
}

namespace agxCollide
{
	class Sphere;
	class Box;
	class Cylinder;
	class Capsule;
	class Trimesh;
}

namespace agxModel
{
	class TwoBodyTire;
}

namespace AGXBarrierFactories
{
	FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body);

	FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere);

	FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box);

	FCylinderShapeBarrier CreateCylinderShapeBarrier(agxCollide::Cylinder* Cylinder);

	FCapsuleShapeBarrier CreateCapsuleShapeBarrier(agxCollide::Capsule* Capsule);

	FTrimeshShapeBarrier CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh);

	FHingeBarrier CreateHingeBarrier(agx::Hinge* Hinge);

	FPrismaticBarrier CreatePrismaticBarrier(agx::Prismatic* Prismatic);

	FBallJointBarrier CreateBallJointBarrier(agx::BallJoint* BallJoint);

	FCylindricalJointBarrier CreateCylindricalJointBarrier(agx::CylindricalJoint* CylindricalJoint);

	FDistanceJointBarrier CreateDistanceJointBarrier(agx::DistanceJoint* DistanceJoint);

	FLockJointBarrier CreateLockJointBarrier(agx::LockJoint* LockJoint);

	FShapeMaterialBarrier CreateShapeMaterialBarrier(agx::Material* Material);

	FContactMaterialBarrier CreateContactMaterialBarrier(agx::ContactMaterial* ContactMaterial);

	FTwoBodyTireBarrier CreateTwoBodyTireBarrier(agxModel::TwoBodyTire* Tire);
}
