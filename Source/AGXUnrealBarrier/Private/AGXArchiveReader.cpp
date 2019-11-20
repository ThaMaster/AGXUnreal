#include "AGXArchiveReader.h"

#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "TypeConversions.h"

#include "AGXBarrierFactories.h"

#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Encoding.h>
#include <agx/Hinge.h>
#include <agx/Prismatic.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/LockJoint.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"


namespace
{
	void InstantiateShapes(const agxCollide::ShapeRefVector& Shapes, FAGXArchiveBody& ArchiveBody)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			switch(Shape->getType())
			{
				case agxCollide::Shape::SPHERE:
				{
					agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
					ArchiveBody.InstantiateSphere(CreateSphereShapeBarrier(Sphere));
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					ArchiveBody.InstantiateBox(CreateBoxShapeBarrier(Box));
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					ArchiveBody.InstantiateTrimesh(CreateTrimeshShapeBarrier(Trimesh));
					break;
				}
				case agxCollide::Shape::GROUP:
				{
					agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
					InstantiateShapes(Group->getChildren(), ArchiveBody);
					break;
				}
			}
		}
	}
}

void FAGXArchiveReader::Read(const FString& Filename, FAGXArchiveInstantiator& Instantiator)
{
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	size_t NumRead {Simulation->read(Convert(Filename))};
	if (NumRead == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Could not read .agx filel %s."), *Filename);
		return;
	}

	agx::RigidBodyRefVector& Bodies {Simulation->getRigidBodies()};
	if (Bodies.size() > size_t(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Log, TEXT(".agx file %s contains too many bodies."), *Filename);
		return; /// \todo Should be bail, or restore as many bodies as we can?
	}

	for (agx::RigidBodyRef& Body : Bodies)
	{
		FRigidBodyBarrier BodyBarrier {CreateRigidBodyBarrier(Body)};
		std::unique_ptr<FAGXArchiveBody> ArchiveBody {Instantiator.InstantiateBody(BodyBarrier)};
		const agxCollide::GeometryRefVector& Geometries {Body->getGeometries()};
		for (const agxCollide::GeometryRef& Geometry : Geometries)
		{
			::InstantiateShapes(Geometry->getShapes(), *ArchiveBody);
		}
	}

	agx::ConstraintRefSetVector& Constraints = Simulation->getConstraints();
	if (Constraints.size() > size_t(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Log, TEXT(".agx file %s contains too many constraints."), *Filename);
		return; /// \todo Should we bail, or restore as many constraints as we can?
	}

	for (agx::ConstraintRef& Constraint : Constraints)
	{
		if (agx::Hinge* Hinge = Constraint->asSafe<agx::Hinge>())
		{
			Instantiator.InstantiateHinge(CreateHingeBarrier(Hinge));
		}
		else if (agx::Prismatic* Prismatic = Constraint->asSafe<agx::Prismatic>())
		{
			Instantiator.InstantiatePrismatic(CreatePrismaticBarrier(Prismatic));
		}
		else if (agx::BallJoint* BallJoint = Constraint->asSafe<agx::BallJoint>())
		{
			Instantiator.InstantiateBallJoint(CreateBallJointBarrier(BallJoint));
		}
		else if (agx::CylindricalJoint* CylindricalJoint = Constraint->asSafe<agx::CylindricalJoint>())
		{
			Instantiator.InstantiateCylindricalJoint(CreateCylindricalJointBarrier(CylindricalJoint));
		}
		else if (agx::DistanceJoint* DistanceJoint = Constraint->asSafe<agx::DistanceJoint>())
		{
			Instantiator.InstantiateDistanceJoint(CreateDistanceJointBarrier(DistanceJoint));
		}
		else if (agx::LockJoint* LockJoint = Constraint->asSafe<agx::LockJoint>())
		{
			Instantiator.InstantiateLockJoint(CreateLockJointBarrier(LockJoint));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Constraint '%s' has unupported type."), *Convert(Constraint->getName()));
		}
	}
}
