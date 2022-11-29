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
#include <agxVehicle/Track.h>
#include "EndAGXIncludes.h"

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
					OutSimObjects.GetSphereShapes().Add(
						AGXBarrierFactories::CreateSphereShapeBarrier(Sphere));
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					OutSimObjects.GetBoxShapes().Add(
						AGXBarrierFactories::CreateBoxShapeBarrier(Box));
					break;
				}
				case agxCollide::Shape::CYLINDER:
				{
					agxCollide::Cylinder* Cylinder {Shape->as<agxCollide::Cylinder>()};
					OutSimObjects.GetCylinderShapes().Add(
						AGXBarrierFactories::CreateCylinderShapeBarrier(Cylinder));
					break;
				}
				case agxCollide::Shape::CAPSULE:
				{
					agxCollide::Capsule* Capsule {Shape->as<agxCollide::Capsule>()};
					OutSimObjects.GetCapsuleShapes().Add(
						AGXBarrierFactories::CreateCapsuleShapeBarrier(Capsule));
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					OutSimObjects.GetTrimeshShapes().Add(
						AGXBarrierFactories::CreateTrimeshShapeBarrier(Trimesh));
					break;
				}
				case agxCollide::Shape::GROUP:
				{
					agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
					ReadShapes(Group->getChildren(), OutSimObjects);
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
			   agxCable::Cable::getCableForBody(&Body) == nullptr &&
			   agxVehicle::Track::get(&Body) == nullptr;
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
			OutSimObjects.GetContactMaterials().Add(
				AGXBarrierFactories::CreateContactMaterialBarrier(ContMat));
		}
	}

	void ReadTireModels(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects, TArray<agx::Constraint*>& NonFreeConstraint)
	{
		const agxSDK::AssemblyHash& Assemblies = Simulation.getAssemblies();

		for (const auto& Assembly : Assemblies)
		{
			agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(Assembly.first);
			if (Tire == nullptr)
			{
				continue;
			}

			if (Tire->getHubRigidBody() == nullptr || Tire->getTireRigidBody() == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Tire '%s' is missing Hub or Tire Rigid Body. It will not be imported."),
					*Convert(Tire->getName()));
				continue;
			}

			// Add the Tire owned Hinge to the list of non-free Constraints. These are used later to
			// avoid duplicate imports of those Constraints.
			NonFreeConstraint.Add(Tire->getHinge());

			OutSimObjects.GetTwoBodyTires().Add(
				AGXBarrierFactories::CreateTwoBodyTireBarrier(Tire));
		}
	}

	void ReadRigidBodies(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects)
	{
		agx::RigidBodyRefVector& Bodies {Simulation.getRigidBodies()};
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

			OutSimObjects.GetRigidBodies().Add(AGXBarrierFactories::CreateRigidBodyBarrier(Body));
		}
	}

	void ReadTracks(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FSimulationObjectCollection& OutSimObjects, TArray<agx::Constraint*>& NonFreeConstraint)
	{
		agxVehicle::TrackPtrVector Tracks = agxVehicle::Track::findAll(&Simulation);

		for (agxVehicle::Track* Track : Tracks)
		{
			if (Track == nullptr || Track->getRoute() == nullptr)
			{
				continue;
			}

			OutSimObjects.GetTracks().Add(AGXBarrierFactories::CreateTrackBarrier(Track));
			const int32 NumNodes = Track->getNumNodes();
			for (int i = 0; i < NumNodes; i++)
			{
				agxVehicle::TrackNode* Node = Track->getNode(i);
				if (Node == nullptr)
					continue;

				if (agx::Constraint* Constraint = Node->getConstraint())
					NonFreeConstraint.Add(Constraint);
			}
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
		FSimulationObjectCollection& OutSimObjects,
		const TArray<agx::Constraint*>& NonFreeConstraint)
	{
		agx::ConstraintRefSetVector& Constraints = Simulation.getConstraints();
		for (agx::ConstraintRef& Constraint : Constraints)
		{
			if (Constraint == nullptr)
			{
				continue;
			}

			if (NonFreeConstraint.Contains(Constraint.get()))
			{
				// This is a non-free constraint, and will be imported by the thing owning it.
				continue;
			}

			if (agx::Hinge* Hinge = Constraint->asSafe<agx::Hinge>())
			{
				OutSimObjects.GetHingeConstraints().Add(
					AGXBarrierFactories::CreateHingeBarrier(Hinge));
			}
			else if (agx::Prismatic* Prismatic = Constraint->asSafe<agx::Prismatic>())
			{
				OutSimObjects.GetPrismaticConstraints().Add(
					AGXBarrierFactories::CreatePrismaticBarrier(Prismatic));
			}
			else if (agx::BallJoint* BallJoint = Constraint->asSafe<agx::BallJoint>())
			{
				OutSimObjects.GetBallConstraints().Add(
					AGXBarrierFactories::CreateBallJointBarrier(BallJoint));
			}
			else if (
				agx::CylindricalJoint* CylindricalJoint =
					Constraint->asSafe<agx::CylindricalJoint>())
			{
				OutSimObjects.GetCylindricalConstraints().Add(
					AGXBarrierFactories::CreateCylindricalJointBarrier(CylindricalJoint));
			}
			else if (agx::DistanceJoint* DistanceJoint = Constraint->asSafe<agx::DistanceJoint>())
			{
				OutSimObjects.GetDistanceConstraints().Add(
					AGXBarrierFactories::CreateDistanceJointBarrier(DistanceJoint));
			}
			else if (agx::LockJoint* LockJoint = Constraint->asSafe<agx::LockJoint>())
			{
				OutSimObjects.GetLockConstraints().Add(
					AGXBarrierFactories::CreateLockJointBarrier(LockJoint));
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

	void ReadWires(agxSDK::Simulation& Simulation, FSimulationObjectCollection& OutSimObjects)
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

	void ReadObserverFrames(
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
		TArray<agx::Constraint*> NonFreeConstraints;

		ReadMaterials(Simulation, OutSimObjects);
		ReadTireModels(Simulation, Filename, OutSimObjects, NonFreeConstraints);
		ReadBodilessGeometries(Simulation, Filename, OutSimObjects);
		ReadRigidBodies(Simulation, Filename, OutSimObjects);
		ReadTracks(Simulation, Filename, OutSimObjects, NonFreeConstraints);
		ReadConstraints(Simulation, Filename, OutSimObjects, NonFreeConstraints);
		ReadCollisionGroups(Simulation, OutSimObjects);
		ReadWires(Simulation, OutSimObjects);
		ReadObserverFrames(Simulation, OutSimObjects);
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

	::ReadAll(*Simulation, Filename, OutSimObjects);
	return true;
}

AGXUNREALBARRIER_API bool FAGXSimObjectsReader::ReadUrdf(
	const FString& UrdfFilePath, const FString& UrdfPackagePath,
	FSimulationObjectCollection& OutSimObjects)
{
#if AGX_VERSION_GREATER_OR_EQUAL(2, 33, 0, 0)
	agxModel::UrdfReader::Settings UrdfSettings(
		/*fixToWorld*/ false, /*disableLinkedBodies*/ false, /*mergeKinematicLinks*/ false);
	agxSDK::AssemblyRef Model = agxModel::UrdfReader::read(
		Convert(UrdfFilePath), Convert(UrdfPackagePath), nullptr, UrdfSettings);
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
	::ReadAll(*Simulation, UrdfFilePath, OutSimObjects);

	return true;
}
