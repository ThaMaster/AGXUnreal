#include "AGXArchiveReader.h"

#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "TypeConversions.h"

#include "AGXBarrierFactories.h"
#include "AGX_LogCategory.h"

#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Encoding.h>
#include <agx/Hinge.h>
#include <agx/Prismatic.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/LockJoint.h>

// In 2.28 including Cable.h causes a preprocessor macro named DEPRECATED to be defined. This
// conflicts with a macro with the same name in Unreal. Undeffing the Unreal one.
/// \todo Remove this #undef once the macro has been removed from AGX Dynamics.
#undef DEPRECATED
#include <agxCable/Cable.h>

#include <agxCollide/Geometry.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include <agxSDK/Simulation.h>
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

namespace
{
	void InstantiateShapes(
		const agxCollide::ShapeRefVector& Shapes,
		FAGXArchiveBody& ArchiveBody)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			switch (Shape->getType())
			{
				case agxCollide::Shape::SPHERE:
				{
					agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
					ArchiveBody.InstantiateSphere(
						AGXBarrierFactories::CreateSphereShapeBarrier(Sphere));
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					ArchiveBody.InstantiateBox(
						AGXBarrierFactories::CreateBoxShapeBarrier(Box));
					break;
				}
				case agxCollide::Shape::CYLINDER:
				{
					agxCollide::Cylinder* Cylinder {Shape->as<agxCollide::Cylinder>()};
					ArchiveBody.InstantiateCylinder(
						AGXBarrierFactories::CreateCylinderShapeBarrier(Cylinder));
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					ArchiveBody.InstantiateTrimesh(
						AGXBarrierFactories::CreateTrimeshShapeBarrier(Trimesh));
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

namespace
{
	bool IsRegularBody(agx::RigidBody& Body)
	{
		return !Body.isPowerlineBody() && agxWire::Wire::getWire(&Body) == nullptr &&
			   agxCable::Cable::getCableForBody(&Body) == nullptr;
	}

	/**
	 * Get all materials and create one shape material asset for each. Each agx::Material have a
	 * unique name. Several agx::Geometries may use the same agx::Material.
	 */
	void RestoreMaterials(agxSDK::Simulation& Simulation, FAGXArchiveInstantiator& Instantiator)
	{
		const agxSDK::StringMaterialRefTable& MaterialsTable =
			Simulation.getMaterialManager()->getMaterials();
		for (auto& It : MaterialsTable)
		{
			agx::Material* Mat = It.second.get();
			Instantiator.InstantiateShapeMaterial(
				AGXBarrierFactories::CreateShapeMaterialBarrier(Mat));
		}

		const agxSDK::MaterialSPairContactMaterialRefTable& ContactMaterialsTable =
			Simulation.getMaterialManager()->getContactMaterials();
		for (auto& It : ContactMaterialsTable)
		{
			agx::ContactMaterial* ContMat = It.second.get();

			Instantiator.InstantiateContactMaterial(
				AGXBarrierFactories::CreateContactMaterialBarrier(ContMat));
		}
	}

	void RestoreRigidBodies(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		agx::RigidBodyRefVector& Bodies {Simulation.getRigidBodies()};
		if (Bodies.size() > size_t(std::numeric_limits<int32>::max()))
		{ /// \todo Are there checks really necessary?
			UE_LOG(LogAGX, Log, TEXT(".agx file %s contains too many bodies."), *Filename);
			return; /// \todo Should we bail, or restore as many bodies as we can?
		}

		for (agx::RigidBodyRef& Body : Bodies)
		{
			if (Body == nullptr)
			{
				continue;
			}
			if (!IsRegularBody(*Body))
			{
				continue;
			}

			FRigidBodyBarrier BodyBarrier {AGXBarrierFactories::CreateRigidBodyBarrier(Body)};
			std::unique_ptr<FAGXArchiveBody> ArchiveBody {
				Instantiator.InstantiateBody(BodyBarrier)};

			const agxCollide::GeometryRefVector& Geometries {Body->getGeometries()};
			for (const agxCollide::GeometryRef& Geometry : Geometries)
			{
#if 0
				FString ShapeMaterialAsset;
				if (agx::Material* AgxMaterial = Geometry->getMaterial())
				{
					check(ShapeMaterialAssets.Find(AgxMaterial));
					ShapeMaterialAsset = ShapeMaterialAssets[AgxMaterial];
				}
#endif
				::InstantiateShapes(Geometry->getShapes(), *ArchiveBody);
			}
		}
	}

	void RestoreConstraints()
	{
	}
}

void FAGXArchiveReader::Read(const FString& Filename, FAGXArchiveInstantiator& Instantiator)
{
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	size_t NumRead {Simulation->read(Convert(Filename))};
	if (NumRead == 0)
	{
		UE_LOG(LogAGX, Log, TEXT("Could not read .agx file %s."), *Filename);
		return;
	}

	::RestoreMaterials(*Simulation, Instantiator);

	::RestoreRigidBodies(*Simulation, Filename, Instantiator);

	agx::ConstraintRefSetVector& Constraints = Simulation->getConstraints();
	if (Constraints.size() > size_t(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogAGX, Log, TEXT(".agx file %s contains too many constraints."), *Filename);
		return; /// \todo Should we bail, or restore as many constraints as we can?
		/// \todo Should we do as much error checking as possible first, before
		/// creating any Editor instances?
	}

	for (agx::ConstraintRef& Constraint : Constraints)
	{
		if (agx::Hinge* Hinge = Constraint->asSafe<agx::Hinge>())
		{
			Instantiator.InstantiateHinge(AGXBarrierFactories::CreateHingeBarrier(Hinge));
		}
		else if (agx::Prismatic* Prismatic = Constraint->asSafe<agx::Prismatic>())
		{
			Instantiator.InstantiatePrismatic(
				AGXBarrierFactories::CreatePrismaticBarrier(Prismatic));
		}
		else if (agx::BallJoint* BallJoint = Constraint->asSafe<agx::BallJoint>())
		{
			Instantiator.InstantiateBallJoint(
				AGXBarrierFactories::CreateBallJointBarrier(BallJoint));
		}
		else if (
			agx::CylindricalJoint* CylindricalJoint = Constraint->asSafe<agx::CylindricalJoint>())
		{
			Instantiator.InstantiateCylindricalJoint(
				AGXBarrierFactories::CreateCylindricalJointBarrier(CylindricalJoint));
		}
		else if (agx::DistanceJoint* DistanceJoint = Constraint->asSafe<agx::DistanceJoint>())
		{
			Instantiator.InstantiateDistanceJoint(
				AGXBarrierFactories::CreateDistanceJointBarrier(DistanceJoint));
		}
		else if (agx::LockJoint* LockJoint = Constraint->asSafe<agx::LockJoint>())
		{
			Instantiator.InstantiateLockJoint(
				AGXBarrierFactories::CreateLockJointBarrier(LockJoint));
		}
		else
		{
			UE_LOG(
				LogAGX, Log, TEXT("Constraint '%s' has unupported type."),
				*Convert(Constraint->getName()));
		}
	}

	agxCollide::CollisionGroupManager* CollisionGroupManager =
		Simulation->getSpace()->getCollisionGroupManager();
	agxCollide::CollisionGroupManager::SymmetricCollisionGroupVector DisabledGroupPairs =
		CollisionGroupManager->getDisabledCollisionGroupPairs();
	TArray<std::pair<FString, FString>> DisabledGroups;
	DisabledGroups.Reserve(static_cast<int32>(DisabledGroupPairs.size()));
	for (agx::SymmetricPair<agx::Physics::CollisionGroupPtr>& Pair : DisabledGroupPairs)
	{
		FString Group1 = Convert(Pair.first->getName());
		FString Group2 = Convert(Pair.second->getName());
		DisabledGroups.Add({Group1, Group2});
	}
	Instantiator.DisabledCollisionGroups(DisabledGroups);
}
