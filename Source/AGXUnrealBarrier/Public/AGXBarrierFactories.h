#pragma once

#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"

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
	class Trimesh;
}

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body);

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere);

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box);

FTrimeshShapeBarrier CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh);

FHingeBarrier CreateHingeBarrier(agx::Hinge* Hinge);

FPrismaticBarrier CreatePrismaticBarrier(agx::Prismatic* Prismatic);

FBallJointBarrier CreateBallJointBarrier(agx::BallJoint* BallJoint);

FCylindricalJointBarrier CreateCylindricalJointBarrier(agx::CylindricalJoint* CylindricalJoint);

FDistanceJointBarrier CreateDistanceJointBarrier(agx::DistanceJoint* DistanceJoint);
