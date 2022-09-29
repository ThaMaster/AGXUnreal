// Copyright 2022, Algoryx Simulation AB.

#include "AGXSimObjectsReader.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXBarrierFactories.h"
#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
#include "SimulationObjectCollection.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/BallJoint.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/Encoding.h>
#include <agx/Hinge.h>
#include <agx/LockJoint.h>
#include <agx/Prismatic.h>
#include <agx/RigidBody.h>
#include <agx/version.h>

// In 2.28 including Cable.h causes a preprocessor macro named DEPRECATED to be defined. This
// conflicts with a macro with the same name in Unreal. Undeffing the Unreal one.
/// \todo Remove this #undef once the macro has been removed from AGX Dynamics.
#undef DEPRECATED
#include <agxCable/Cable.h>

#include <agxCollide/Box.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Trimesh.h>
#include <agxModel/TwoBodyTire.h>
#include <agxModel/UrdfReader.h>
#include <agxSDK/Simulation.h>
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "AGX_SIM_OBJECTS_READER"

namespace
{
	void ReadShapes(
		const agxCollide::ShapeRefVector& Shapes, FSimulationObjectCollection& OutSimObjects)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			switch (Shape->getType())
			{
				case agxCollide::Shape::SPHERE:
				{
					agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
					OutSimObjects.GetShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Sphere));
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					OutSimObjects.GetShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Box));
					break;
				}
				case agxCollide::Shape::CYLINDER:
				{
					agxCollide::Cylinder* Cylinder {Shape->as<agxCollide::Cylinder>()};
					OutSimObjects.GetShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Cylinder));
					break;
				}
				case agxCollide::Shape::CAPSULE:
				{
					agxCollide::Capsule* Capsule {Shape->as<agxCollide::Capsule>()};
					OutSimObjects.GetShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Capsule));
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					OutSimObjects.GetShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Trimesh));
					break;
				}
				case agxCollide::Shape::GROUP:
				{
					agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
					InstantiateShapes(Group->getChildren(), OutSimObjects);
					break;
				}
			}
		}
	}
}

namespace
{
	bool IsRegularBody(agx::RigidBody& Body, const TArray<agx::RigidBody*>& NonFreeBodies)
	{
		return !Body.isPowerlineBody() && agxWire::Wire::getWire(&Body) == nullptr &&
			   agxCable::Cable::getCableForBody(&Body) == nullptr && !NonFreeBodies.Contains(Body);
	}

	/**
	 * Several agx::Geometries may use the same agx::Material.
	 */
	void ReadMaterials(agxSDK::Simulation& Simulation, FSimulationObjectCollection& OutSimObjects)
	{
		const agxSDK::StringMaterialRefTable& MaterialsTable =
			Simulation.getMaterialManager()->getMaterials();
		OutSimObjects.GetShapeMaterials().Reserve(MaterialsTable.size());
		for (auto& It : MaterialsTable)
		{
			agx::Material* Mat = It.second.get();
			OutSimObjects.GetShapeMaterials().Add(
				AGXBarrierFactories::CreateShapeMaterialBarrier(Mat));
		}

		const agxSDK::MaterialSPairContactMaterialRefTable& ContactMaterialsTable =
			Simulation.getMaterialManager()->getContactMaterials();
		OutSimObjects.GetContactMaterials().Reserve(ContactMaterialsTable.size());
		for (auto& It : ContactMaterialsTable)
		{
			agx::ContactMaterial* ContMat = It.second.get();
			OutSimObjects.ContactMaterials().Add(
				AGXBarrierFactories::CreateContactMaterialBarrier(ContMat));
		}
	}

	void ReadTireModels(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects, TArray<agx::RigidBody*>& NonFreeBodies)
	{
		const agxSDK::AssemblyHash& Assemblies = Simulation.getAssemblies();

		for (const auto& Assembly : Assemblies)
		{
			agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(Assembly.first);
			if (Tire == nullptr)
			{
				continue;
			}

			OutSimObjects.GetTires().Add(AGXBarrierFactories::CreateTwoBodyTireBarrier(Tire));

			if (agx::RigidBody* Body = Tire->getTireRigidBody())
			{
				NonFreeBodies.Add(Body);
			}

			if (agx::RigidBody* Body = Tire->getHubRigidBody())
			{
				NonFreeBodies.Add(Body);
			}
		}
	}

	// The NonFreeBodies must be complete before calling this function for the filtering to work.
	void ReadRigidBodies(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects, const TArray<agx::RigidBody*>& NonFreeBodies)
	{
		agx::RigidBodyRefVector& Bodies {Simulation.getRigidBodies()};
		for (agx::RigidBodyRef& Body : Bodies)
		{
			if (Body == nullptr)
			{
				continue;
			}
			if (!IsRegularBody(*Body, NonFreeBodies))
			{
				continue;
			}

			OutSimObjects.Add(AGXBarrierFactories::CreateRigidBodyBarrier(Body));
		}
	}

	// Reads and instantiates all Geometries not owned by a RigidBody.
	void ReadBodilessGeometries(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects)
	{
		const agxCollide::GeometryRefVector& Geometries = Simulation.getGeometries();

		for (const agxCollide::GeometryRef& Geometry : Geometries)
		{
			if (Geometry == nullptr || Geometry->getRigidBody() != nullptr)
			{
				continue;
			}

			::ReadShapes(Geometry->getShapes(), OutSimObjects);
		}
	}

	void ReadConstraints(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects)
	{
		agx::ConstraintRefSetVector& Constraints = Simulation.getConstraints();
		OutSimObjects.GetConstraints().Reserve(Constraints.size());
		for (agx::ConstraintRef& Constraint : Constraints)
		{
			if (agx::Hinge* Hinge = Constraint->asSafe<agx::Hinge>())
			{
				OutSimObjects.Add(AGXBarrierFactories::CreateHingeBarrier(Hinge));
			}
			else if (agx::Prismatic* Prismatic = Constraint->asSafe<agx::Prismatic>())
			{
				OutSimObjects.Add(AGXBarrierFactories::CreatePrismaticBarrier(Prismatic));
			}
			else if (agx::BallJoint* BallJoint = Constraint->asSafe<agx::BallJoint>())
			{
				OutSimObjects.Add(AGXBarrierFactories::CreateBallJointBarrier(BallJoint));
			}
			else if (
				agx::CylindricalJoint* CylindricalJoint =
					Constraint->asSafe<agx::CylindricalJoint>())
			{
				OutSimObjects.Add(
					AGXBarrierFactories::CreateCylindricalJointBarrier(CylindricalJoint));
			}
			else if (agx::DistanceJoint* DistanceJoint = Constraint->asSafe<agx::DistanceJoint>())
			{
				OutSimObjects.Add(AGXBarrierFactories::CreateDistanceJointBarrier(DistanceJoint));
			}
			else if (agx::LockJoint* LockJoint = Constraint->asSafe<agx::LockJoint>())
			{
				OutSimObjects.Add(AGXBarrierFactories::CreateLockJointBarrier(LockJoint));
			}
		}
	}

	void ReadCollisionGroups(
		agxSDK::Simulation& Simulation, FSimulationObjectCollection& OutSimObjects)
	{
		auto GetCollisionGroupString = [](const agx::Physics::CollisionGroupPtr& Cg) -> FString
		{
			FString Str = Convert(Cg.name());

			// If the CollisionGroup was stored as an Id (uint32), then it will contain no name
			// data.
			if (!Str.IsEmpty())
			{
				return Str;
			}

			return FString::FromInt(Cg.id());
		};

		agxCollide::CollisionGroupManager* CollisionGroupManager =
			Simulation.getSpace()->getCollisionGroupManager();
		agxCollide::CollisionGroupManager::SymmetricCollisionGroupVector DisabledGroupPairs =
			CollisionGroupManager->getDisabledCollisionGroupPairs();

		OutSimObjects.GetDisabledCollisionGroups().Reserve(
			static_cast<int32>(DisabledGroupPairs.size()));
		for (agx::SymmetricPair<agx::Physics::CollisionGroupPtr>& Pair : DisabledGroupPairs)
		{
			FString Group1 = GetCollisionGroupString(Pair.first);
			FString Group2 = GetCollisionGroupString(Pair.second);
			OutSimObjects.GetDisabledCollisionGroups().Add({Group1, Group2});
		}
	}

	bool ReadWires(agxSDK::Simulation& Simulation, FSimulationObjectCollection& OutSimObjects)
	{
		agxWire::WirePtrVector Wires = agxWire::Wire::findAll(&Simulation);
		OutSimObjects.GetWires().Reserve(Wires.size());
		for (agxWire::Wire* Wire : Wires)
		{
			if (Wire == nullptr)
			{
				continue;
			}

			OutSimObjects.GetWires().Add(AGXBarrierFactories::CreateWireBarrier(Wire));
		}
	}

	bool ReadObserverFrames(
		agxSDK::Simulation& Simulation, FSimulationObjectCollection& OutSimObjects)
	{
		const agx::ObserverFrameRefSetVector& ObserverFrames = Simulation.getObserverFrames();
		OutSimObjects.GetObserverFrames().Reserve(ObserverFrames.size());
		for (const agx::ObserverFrameRef& ObserverFrame : ObserverFrames)
		{
			const FString Name = Convert(ObserverFrame->getName());
			const FGuid BodyGuid = Convert(ObserverFrame->getRigidBody()->getUuid());
			const FTransform Transform = Convert(ObserverFrame->getLocalTransform());
			OutSimObjects.GetObserverFrames().Add({Name, BodyGuid, Transform});
		}
	}

	void ReadAll(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects)
	{
		TArray<agx::RigidBody*> NonFreeBodies;

		ReadMaterials(Simulation, OutSimObjects);
		ReadTireModels(Simulation, Filename, OutSimObjects, NonFreeBodies);
		ReadBodilessGeometries(Simulation, Filename, OutSimObjects);
		ReadConstraints(Simulation, Filename, OutSimObjects);
		ReadCollisionGroups(Simulation, OutSimObjects);
		ReadWires(Simulation, OutSimObjects);
		ReadObserverFrames(Simulation, OutSimObjects);

		// We read Rigid Bodies last so that the NonFreeBodies Array is complete.
		// Any Body within that Array are ignored.
		ReadRigidBodies(Simulation, Filename, OutSimObjects, NonFreeBodies);
	}
}

bool FAGXSimObjectsReader::ReadAGXArchive(
	const FString& Filename, FSimulationObjectCollection& OutSimObjects)
{
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	try
	{
		size_t NumRead = Simulation->read(Convert(Filename));
		if (NumRead == 0)
		{
			UE_LOG(LogAGX, Error, TEXT("Could not read .agx file '%s'."), *Filename);
			return false;
		}
	}
	catch (const std::runtime_error& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not read .agx file '%s':\n\n%s"), *Filename, *Error.what());
		return false;
	}

	if (::ReadAll(*Simulation, Filename, OutSimObjects) == false)
	{
		// Logging done in ReadAll().
		return false;
	}

	return true;
}

AGXUNREALBARRIER_API bool FAGXSimObjectsReader::ReadUrdf(
	const FString& UrdfFilePath, const FString& UrdfPackagePath,
	FSimulationObjectCollection& OutSimObjects)
{
	ImportTask.EnterProgressFrame(WorkRead, FText::FromString("Reading URDF file"));
#if AGX_VERSION_GREATER_OR_EQUAL(2, 33, 0, 0)
	agxSDK::AssemblyRef Model =
		agxModel::UrdfReader::read(Convert(UrdfFilePath), Convert(UrdfPackagePath), nullptr);
#else
	agxSDK::AssemblyRef Model = agxModel::UrdfReader::read(
		Convert(UrdfFilePath), Convert(UrdfPackagePath), nullptr, /*fixToWorld*/ false);
#endif

	if (Model == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not read URDF file '%s'. The Log category LogAGXDynamics may include more "
				 "details."),
			*UrdfFilePath);
		return false;
	}

	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	Simulation->add(Model);

	if (::ReadAll(*Simulation, UrdfFilePath, OutSimObjects) == false)
	{
		// Logging done in ReadAll().
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
