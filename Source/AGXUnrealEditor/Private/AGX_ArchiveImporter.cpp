#include "AGX_ArchiveImporter.h"

// AGXUnreal includes.
#include "AGXArchiveReader.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "RigidBodyBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/LockJointBarrier.h"

#include "AGX_CollisionGroupManager.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "AGX_LogCategory.h"

#include "Constraints/AGX_HingeConstraint.h"
#include "Constraints/AGX_PrismaticConstraint.h"
#include "Constraints/AGX_BallConstraint.h"
#include "Constraints/AGX_CylindricalConstraint.h"
#include "Constraints/AGX_DistanceConstraint.h"
#include "Constraints/AGX_LockConstraint.h"
#include "Constraints/ControllerConstraintBarriers.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Internationalization/Text.h"
#include "Math/Transform.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

#include <iostream>

namespace
{
	std::tuple<AActor*, USceneComponent*> InstantiateBody(
		const FRigidBodyBarrier& Body, UWorld& World)
	{
		UE_LOG(LogAGX, Log, TEXT("Loaded AGX body with name '%s'."), *Body.GetName());
		AActor* NewActor;
		USceneComponent* Root;
		FTransform Transform(Body.GetRotation(), Body.GetPosition());
		std::tie(NewActor, Root) = FAGX_EditorUtilities::CreateEmptyActor(Transform, &World);
		if (NewActor == nullptr)
		{
			UE_LOG(LogAGX, Log, TEXT("Could not create Actor for body '%s'."), *Body.GetName());
			return {nullptr, nullptr};
		}
		if (Root == nullptr)
		{
			/// \todo Do we need to destroy the Actor here?
			UE_LOG(
				LogAGX, Log, TEXT("Could not create SceneComponent for body '%s'."),
				*Body.GetName());
			return {nullptr, nullptr};
		}

		NewActor->SetActorLabel(Body.GetName());

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		if (NewBody == nullptr)
		{
			UE_LOG(
				LogAGX, Log, TEXT("Could not create AGX RigidBodyComponenet for body '%s'."),
				*Body.GetName());
			/// \todo Do we need to destroy the Actor and the RigidBodyComponent here?
			return {nullptr, nullptr};
		}

		NewBody->Rename(TEXT("AGX_RigidBodyComponent"));
		NewBody->Mass = Body.GetMass();
		NewBody->MotionControl = Body.GetMotionControl();

		return {NewActor, Root};
	}

	// Archive instantiator that creates sub-objects under RigidBody. Knows how
	// to create various subclasses of AGX_ShapesComponent in Unreal Editor.
	class EditorBody final : public FAGXArchiveBody
	{
	public:
		EditorBody(
			AActor& InActor, USceneComponent& InRoot, UWorld& InWorld, const FString& InArchiveName)
			: Actor(InActor)
			, Root(InRoot)
			, World(InWorld)
			, ArchiveName(InArchiveName)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Sphere) override
		{
			UAGX_SphereShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateSphereShape(&Actor, &Root);
			ShapeComponent->Radius = Sphere.GetRadius();
			FinalizeShape(ShapeComponent, Sphere);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Box) override
		{
			UAGX_BoxShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateBoxShape(&Actor, &Root);
			ShapeComponent->HalfExtent = Box.GetHalfExtents();
			FinalizeShape(ShapeComponent, Box);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Trimesh) override
		{
			UAGX_TrimeshShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateTrimeshShape(&Actor, &Root);
			ShapeComponent->MeshSourceLocation =
				EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
			UStaticMeshComponent* MeshComponent = FAGX_EditorUtilities::CreateStaticMeshAsset(
				&Actor, ShapeComponent, Trimesh, ArchiveName);
			ShapeComponent->Rename(*MeshComponent->GetName());
			FinalizeShape(ShapeComponent, Trimesh);
		}

	private:
		void FinalizeShape(UAGX_ShapeComponent* Component, const FShapeBarrier& Barrier)
		{
			Component->bCanCollide = Barrier.GetEnableCollisions();
			for (const FName& Group : Barrier.GetCollisionGroups())
			{
				Component->AddCollisionGroup(Group);
			}

			FVector Location;
			FQuat Rotation;
			std::tie(Location, Rotation) = Barrier.GetLocalPositionAndRotation();
			Component->SetRelativeLocationAndRotation(Location, Rotation);
			Component->UpdateVisualMesh();
		}

		AActor& Actor;
		USceneComponent& Root;
		UWorld& World;
		const FString& ArchiveName;
	};

	// Archive instantiator that creates top-level objects. Knows how to create
	// UAGX_RigidBodyComponent in Unreal Editor.
	class EditorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		EditorInstantiator(AActor& InImportedRoot, UWorld& InWorld)
			: ImportedRoot(InImportedRoot)
			, World(InWorld)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			AActor* NewActor;
			USceneComponent* ActorRoot;
			std::tie(NewActor, ActorRoot) = ::InstantiateBody(Barrier, World);
			if (NewActor == nullptr || ActorRoot == nullptr)
			{
				return nullptr;
			}
			NewActor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			Bodies.Add(Barrier.GetGuid(), NewActor);
			return new EditorBody(*NewActor, *ActorRoot, World, ImportedRoot.GetActorLabel());
		}

		virtual void InstantiateHinge(const FHingeBarrier& Hinge) override
		{
			CreateConstraint1DOF(Hinge, AAGX_HingeConstraint::StaticClass());
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Prismatic) override
		{
			CreateConstraint1DOF(Prismatic, AAGX_PrismaticConstraint::StaticClass());
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& BallJoint) override
		{
			CreateConstraint<AAGX_BallConstraint>(BallJoint, AAGX_BallConstraint::StaticClass());
		}

		virtual void InstantiateCylindricalJoint(
			const FCylindricalJointBarrier& CylindricalJoint) override
		{
			CreateConstraint2DOF(CylindricalJoint, AAGX_CylindricalConstraint::StaticClass());
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& DistanceJoint) override
		{
			CreateConstraint1DOF(DistanceJoint, AAGX_DistanceConstraint::StaticClass());
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& LockJoint) override
		{
			CreateConstraint<AAGX_LockConstraint>(LockJoint, AAGX_LockConstraint::StaticClass());
		}

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledGroups) override
		{
			if (DisabledGroups.Num() == 0)
			{
				// Do not force a CollisionGroupManager if there are no disabled groups.
				return;
			}

			// Make sure we have a CollisionGroupManager.
			AAGX_CollisionGroupManager* Manager = AAGX_CollisionGroupManager::GetFrom(&World);
			if (Manager == nullptr)
			{
				Manager = World.SpawnActor<AAGX_CollisionGroupManager>();
			}
			if (Manager == nullptr)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Cannot import disabled collision group pairs because there is no "
						 "CollisionGroupManager in the level."));
				return;
			}

			// Apply the disabled group pairs to the CollisionGroupManager.
			for (auto& Pair : DisabledGroups)
			{
				FName Group1 = *Pair.first;
				FName Group2 = *Pair.second;
				Manager->DisableCollisionGroupPair(Group1, Group2);
			}
		}

	private:
		void CreateConstraint1DOF(const FConstraint1DOFBarrier& Barrier, UClass* ConstraintType)
		{
			AAGX_Constraint1DOF* Constraint =
				CreateConstraint<AAGX_Constraint1DOF>(Barrier, ConstraintType);
			if (Constraint == nullptr)
			{
				// No need to log here, done by CreateConstraint.
				return;
			}

			StoreElectricMotorController(Barrier, Constraint->ElectricMotorController);
			StoreFrictionController(Barrier, Constraint->FrictionController);
			StoreLockController(Barrier, Constraint->LockController);
			StoreRangeController(Barrier, Constraint->RangeController);
			StoreTargetSpeedController(Barrier, Constraint->TargetSpeedController);
		}

		void CreateConstraint2DOF(const FConstraint2DOFBarrier& Barrier, UClass* ConstraintType)
		{
			AAGX_Constraint2DOF* Constraint =
				CreateConstraint<AAGX_Constraint2DOF>(Barrier, ConstraintType);
			if (Constraint == nullptr)
			{
				// No need to log here, done by CreateConstraint.
				return;
			}

			StoreElectricMotorController(Barrier, Constraint->ElectricMotorController1, 0);
			StoreElectricMotorController(Barrier, Constraint->ElectricMotorController2, 1);

			StoreFrictionController(Barrier, Constraint->FrictionController1, 0);
			StoreFrictionController(Barrier, Constraint->FrictionController2, 1);

			StoreLockController(Barrier, Constraint->LockController1, 0);
			StoreLockController(Barrier, Constraint->LockController2, 1);

			StoreRangeController(Barrier, Constraint->RangeController1, 0);
			StoreRangeController(Barrier, Constraint->RangeController2, 1);

			StoreTargetSpeedController(Barrier, Constraint->TargetSpeedController1, 0);
			StoreTargetSpeedController(Barrier, Constraint->TargetSpeedController2, 1);
		}

		template <typename ConstraintType>
		ConstraintType* CreateConstraint(const FConstraintBarrier& Barrier, UClass* ConstraintClass)
		{
			std::pair<AActor*, AActor*> Actors = GetActors(Barrier);
			if (Actors.first == nullptr)
			{
				// Not having a second body is fine. Means that the first body
				// is constrainted to the world. Not having a first body is bad.
				UE_LOG(
					LogAGX, Log, TEXT("Constraint %s doesn't have a first body. Ignoring."),
					*Barrier.GetName());
				return nullptr;
			}

			ConstraintType* Constraint = FAGX_EditorUtilities::CreateConstraint<ConstraintType>(
				Actors.first, Actors.second,
				/*bSelect*/ false, /*bShwNotification*/ false, /*bInPlayingWorld*/ false,
				ConstraintClass);

			StoreFrames(Barrier, *Constraint);

			/// \todo Is there a correct transform here? Does it matter?
			Constraint->SetActorTransform(Actors.first->GetActorTransform());
			Constraint->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			if (!Barrier.GetName().IsEmpty())
			{
				Constraint->Rename(*Barrier.GetName());
				Constraint->SetActorLabel(*Barrier.GetName());
			}

			return Constraint;
		}

		AActor* GetActor(const FRigidBodyBarrier& Body)
		{
			if (!Body.HasNative())
			{
				// No log since not an error. Means constrainted with world.
				return nullptr;
			}
			FGuid Guid = Body.GetGuid();
			AActor* Actor = Bodies.FindRef(Guid);
			if (Actor == nullptr)
			{
				UE_LOG(
					LogAGX, Log,
					TEXT("Found a constraint to body '%s', but that body isn't known."),
					*Body.GetName());
				return nullptr;
			}
			return Actor;
		}

		std::pair<AActor*, AActor*> GetActors(const FConstraintBarrier& Barrier)
		{
			return {GetActor(Barrier.GetFirstBody()), GetActor(Barrier.GetSecondBody())};
		}

		void StoreFrame(
			const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment,
			int32 BodyIndex)
		{
			Attachment.FrameDefiningActor = nullptr;
			Attachment.LocalFrameLocation = Barrier.GetLocalLocation(BodyIndex);
			Attachment.LocalFrameRotation = Barrier.GetLocalRotation(BodyIndex);
		}

		void StoreFrames(const FConstraintBarrier& Barrier, AAGX_Constraint& Constraint)
		{
			StoreFrame(Barrier, Constraint.BodyAttachment1, 0);
			StoreFrame(Barrier, Constraint.BodyAttachment2, 1);
		}

		void StoreElectricMotorController(
			const FConstraint1DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller)
		{
			FElectricMotorControllerBarrier ControllerBarrier;
			Barrier.GetElectricMotorController(ControllerBarrier);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreElectricMotorController(
			const FConstraint2DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller, int32 SecondaryConstraintIndex)
		{
			FElectricMotorControllerBarrier ControllerBarrier;
			Barrier.GetElectricMotorController(ControllerBarrier, SecondaryConstraintIndex);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreFrictionController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller)
		{
			FFrictionControllerBarrier ControllerBarrier;
			Barrier.GetFrictionController(ControllerBarrier);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreFrictionController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
			int32 SecondaryConstraintIndex)
		{
			FFrictionControllerBarrier ControllerBarrier;
			Barrier.GetFrictionController(ControllerBarrier, SecondaryConstraintIndex);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreLockController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller)
		{
			FLockControllerBarrier ControllerBarrier;
			Barrier.GetLockController(ControllerBarrier);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreLockController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
			int32 SecondaryConstraintIndex)
		{
			FLockControllerBarrier ControllerBarrier;
			Barrier.GetLockController(ControllerBarrier, SecondaryConstraintIndex);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreRangeController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller)
		{
			FRangeControllerBarrier ControllerBarrier;
			Barrier.GetRangeController(ControllerBarrier);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreRangeController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
			int32 SecondaryConstraintIndex)
		{
			FRangeControllerBarrier ControllerBarrier;
			Barrier.GetRangeController(ControllerBarrier, SecondaryConstraintIndex);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreTargetSpeedController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller)
		{
			FTargetSpeedControllerBarrier ControllerBarrier;
			Barrier.GetTargetSpeedController(ControllerBarrier);
			Controller.FromBarrier(ControllerBarrier);
		}

		void StoreTargetSpeedController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
			int32 SecondaryConstraintIndex)
		{
			FTargetSpeedControllerBarrier ControllerBarrier;
			Barrier.GetTargetSpeedController(ControllerBarrier, SecondaryConstraintIndex);
			Controller.FromBarrier(ControllerBarrier);
		}

	private:
		AActor& ImportedRoot;
		UWorld& World;

		// Map from Guid/Uuid of the AGX Dynamics body provided by the FAGXArchiveReader to the
		// AActor that we created for that body.
		TMap<FGuid, AActor*> Bodies;
	};

}

AActor* AGX_ArchiveImporter::ImportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	// The ImportGroup AActor will contain all objects created while reading
	// the AGX Dynamics archive.
	AActor* ImportGroup;
	USceneComponent* ImportRoot;
	/// \todo Consider placing ImportedRoot at the center if the imported bodies.
	std::tie(ImportGroup, ImportRoot) =
		FAGX_EditorUtilities::CreateEmptyActor(FTransform::Identity, World);
	if (ImportGroup == nullptr || ImportRoot == nullptr)
	{
		return nullptr;
	}

	FString Filename;
	/// \todo What about platforms that don't use / as a path separator?
	ArchivePath.Split(TEXT("/"), nullptr, &Filename, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	ImportGroup->SetActorLabel(Filename);

	EditorInstantiator Instantiator(*ImportGroup, *World);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE
