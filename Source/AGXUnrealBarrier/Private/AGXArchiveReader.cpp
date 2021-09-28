#include "AGXArchiveReader.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXBarrierFactories.h"
#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
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

#define LOCTEXT_NAMESPACE "AGX_ARCHIVE_READER"

namespace
{
	void InstantiateShapes(
		const agxCollide::ShapeRefVector& Shapes, FAGXArchiveInstantiator& Instantiator,
		FAGXArchiveBody* ArchiveBody = nullptr)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			switch (Shape->getType())
			{
				case agxCollide::Shape::SPHERE:
				{
					agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
					Instantiator.InstantiateSphere(
						AGXBarrierFactories::CreateSphereShapeBarrier(Sphere), ArchiveBody);
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					Instantiator.InstantiateBox(
						AGXBarrierFactories::CreateBoxShapeBarrier(Box), ArchiveBody);
					break;
				}
				case agxCollide::Shape::CYLINDER:
				{
					agxCollide::Cylinder* Cylinder {Shape->as<agxCollide::Cylinder>()};
					Instantiator.InstantiateCylinder(
						AGXBarrierFactories::CreateCylinderShapeBarrier(Cylinder), ArchiveBody);
					break;
				}
				case agxCollide::Shape::CAPSULE:
				{
					agxCollide::Capsule* Capsule {Shape->as<agxCollide::Capsule>()};
					Instantiator.InstantiateCapsule(
						AGXBarrierFactories::CreateCapsuleShapeBarrier(Capsule), ArchiveBody);
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					Instantiator.InstantiateTrimesh(
						AGXBarrierFactories::CreateTrimeshShapeBarrier(Trimesh), ArchiveBody);
					break;
				}
				case agxCollide::Shape::GROUP:
				{
					agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
					InstantiateShapes(Group->getChildren(), Instantiator, ArchiveBody);
					break;
				}
			}
		}
	}

	void InstantiateShapesInBody(
		agx::RigidBody* Body, FAGXArchiveBody& ArchiveBody, FAGXArchiveInstantiator& Instantiator)
	{
		if (Body == nullptr)
		{
			return;
		}

		const agxCollide::GeometryRefVector& Geometries {Body->getGeometries()};
		for (const agxCollide::GeometryRef& Geometry : Geometries)
		{
			::InstantiateShapes(Geometry->getShapes(), Instantiator, &ArchiveBody);
		}
	}
}

namespace
{
	bool VerifyImportSize(size_t Actual, size_t Limit, const FString& Filename, const FString& Type)
	{
		if (Actual > Limit)
		{
			UE_LOG(
				LogAGX, Error, TEXT("AGX Dynamics archive '%s' contains too many %s."), *Filename,
				*Type);
			return false;
		}

		return true;
	}

	bool IsRegularBody(agx::RigidBody& Body)
	{
		return !Body.isPowerlineBody() && agxWire::Wire::getWire(&Body) == nullptr &&
			   agxCable::Cable::getCableForBody(&Body) == nullptr;
	}

	/**
	 * Get all materials and create one shape material asset for each. Each agx::Material have a
	 * unique name. Several agx::Geometries may use the same agx::Material.
	 */
	void ReadMaterials(agxSDK::Simulation& Simulation, FAGXArchiveInstantiator& Instantiator)
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

	void ReadTireModels(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		const agxSDK::AssemblyHash& Assemblies = Simulation.getAssemblies();
		if (!VerifyImportSize(
				Assemblies.size(), std::numeric_limits<int32>::max(), Filename, "assemblies"))
		{
			return;
		}

		auto CheckBody = [](agx::RigidBody* Body, agxModel::Tire* Tire, const FString& Description)
		{
			if (Body == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("The %s used by agxModel::TwoBodyTire: %s "
						 "was nullptr. The agxModel::TwoBodyTire will not be imported."),
					*Description, *Convert(Tire->getName()));
				return false;
			}
			return true;
		};

		for (const auto& Assembly : Assemblies)
		{
			agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(Assembly.first);
			if (Tire == nullptr)
			{
				continue;
			}

			if (!CheckBody(Tire->getTireRigidBody(), Tire, FString("Tire Rigid Body")) ||
				!CheckBody(Tire->getHubRigidBody(), Tire, FString("Hub Rigid Body")))
			{
				continue;
			}

			FTwoBodyTireArchiveBodies ArchiveBodies = Instantiator.InstantiateTwoBodyTire(
				AGXBarrierFactories::CreateTwoBodyTireBarrier(Tire));

			if (ArchiveBodies.TireBodyArchive)
			{
				::InstantiateShapesInBody(
					Tire->getTireRigidBody(), *ArchiveBodies.TireBodyArchive, Instantiator);
			}

			if (ArchiveBodies.HubBodyArchive)
			{
				::InstantiateShapesInBody(
					Tire->getHubRigidBody(), *ArchiveBodies.HubBodyArchive, Instantiator);
			}
		}
	}

	void ReadRigidBodies(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		agx::RigidBodyRefVector& Bodies {Simulation.getRigidBodies()};
		if (!VerifyImportSize(Bodies.size(), std::numeric_limits<int32>::max(), Filename, "bodies"))
		{
			return;
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

			if (ArchiveBody)
			{
				::InstantiateShapesInBody(Body, *ArchiveBody, Instantiator);
			}
		}
	}

	// Reads and instantiates all Geometries not owned by a RigidBody.
	void ReadBodilessGeometries(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		const agxCollide::GeometryRefVector& Geometries = Simulation.getGeometries();
		if (!VerifyImportSize(
				Geometries.size(), std::numeric_limits<int32>::max(), Filename, "geometries"))
		{
			return;
		}

		for (const agxCollide::GeometryRef& Geometry : Geometries)
		{
			if (Geometry == nullptr || Geometry->getRigidBody() != nullptr)
			{
				continue;
			}

			::InstantiateShapes(Geometry->getShapes(), Instantiator);
		}
	}

	void ReadConstraints(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		agx::ConstraintRefSetVector& Constraints = Simulation.getConstraints();
		if (!VerifyImportSize(
				Constraints.size(), std::numeric_limits<int32>::max(), Filename, "constraints"))
		{
			return;
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
				agx::CylindricalJoint* CylindricalJoint =
					Constraint->asSafe<agx::CylindricalJoint>())
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
	}

	void ReadCollisionGroups(agxSDK::Simulation& Simulation, FAGXArchiveInstantiator& Instantiator)
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
		TArray<std::pair<FString, FString>> DisabledGroups;
		DisabledGroups.Reserve(static_cast<int32>(DisabledGroupPairs.size()));
		for (agx::SymmetricPair<agx::Physics::CollisionGroupPtr>& Pair : DisabledGroupPairs)
		{
			FString Group1 = GetCollisionGroupString(Pair.first);
			FString Group2 = GetCollisionGroupString(Pair.second);
			DisabledGroups.Add({Group1, Group2});
		}
		Instantiator.DisabledCollisionGroups(DisabledGroups);
	}

	void ReadWires(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		agxWire::WirePtrVector Wires = agxWire::Wire::findAll(&Simulation);
		for (agxWire::Wire* Wire : Wires)
		{
			if (Wire == nullptr)
			{
				continue;
			}

			FWireBarrier Barrier = AGXBarrierFactories::CreateWireBarrier(Wire);
			Instantiator.InstantiateWire(Barrier);
		}
	}

	void ReadAll(
		agxSDK::Simulation& Simulation, const FString& Filename,
		FAGXArchiveInstantiator& Instantiator)
	{
		const float AmountOfWork = 7.0f; // Keep up to date with the number of /Read.+/-calls.
		FScopedSlowTask MyTask(
			AmountOfWork, LOCTEXT("CreateAGXObjects", "Create AGX Dynamics for Unreal objects"),
			true);
		MyTask.MakeDialog();

		// TODO BEFORE MERGE: add return type indicating success or fail and return it from this
		// function.

		MyTask.EnterProgressFrame(1.0f);
		ReadMaterials(Simulation, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadTireModels(Simulation, Filename, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadRigidBodies(Simulation, Filename, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadBodilessGeometries(Simulation, Filename, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadConstraints(Simulation, Filename, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadCollisionGroups(Simulation, Instantiator);

		MyTask.EnterProgressFrame(1.0f);
		ReadWires(Simulation, Filename, Instantiator);
	}
}

FSuccessOrError FAGXArchiveReader::Read(
	const FString& Filename, FAGXArchiveInstantiator& Instantiator)
{
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	try
	{
		size_t NumRead = Simulation->read(Convert(Filename));
		if (NumRead == 0)
		{
			return FSuccessOrError(
				FString::Printf(TEXT("Could not read .agx file '%s'."), *Filename));
		}
	}
	catch (const std::runtime_error& Error)
	{
		FString What = Error.what();
		return FSuccessOrError(
			FString::Printf(TEXT("Could not read .agx file '%s':\n\n%s"), *Filename, *What));
	}

	::ReadAll(*Simulation, Filename, Instantiator);
	return FSuccessOrError(true);
}

AGXUNREALBARRIER_API FSuccessOrError FAGXArchiveReader::ReadUrdf(
	const FString& UrdfFilePath, const FString& UrdfPackagePath,
	FAGXArchiveInstantiator& Instantiator)
{
	agxSDK::AssemblyRef Model =
		agxModel::UrdfReader::read(Convert(UrdfFilePath), Convert(UrdfPackagePath), nullptr, /*fixToWorld*/ false);

	if (Model == nullptr)
	{
		return FSuccessOrError(FString::Printf(
			TEXT("Could not read URDF file '%s'. The Log category LogAGXDynamics may include more "
				 "details."),
			*UrdfFilePath));
	}

	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	Simulation->add(Model);

	::ReadAll(*Simulation, UrdfFilePath, Instantiator);

	return FSuccessOrError(true);
}

#undef LOCTEXT_NAMESPACE
