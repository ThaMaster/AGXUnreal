#include "AGX_ArchiveImporterToActorTree.h"

// AGXUnreal includes.
#include "AGX_ArchiveImporter.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGXArchiveReader.h"
#include "CollisionGroups/AGX_CollisionGroupManager.h"
/// \todo Creating constraint actors for now, but will switch to creating
/// components when we do import to Blueprint.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_HingeConstraintActor.h"
#include "Constraints/AGX_PrismaticConstraintActor.h"
#include "Constraints/AGX_BallConstraintActor.h"
#include "Constraints/AGX_CylindricalConstraintActor.h"
#include "Constraints/AGX_DistanceConstraintActor.h"
#include "Constraints/AGX_LockConstraintActor.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

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
	// Archive instantiator that creates sub-objects under RigidBody. Knows how
	// to create various subclasses of AGX_ShapesComponent in Unreal Editor.
	class EditorBody final : public FAGXArchiveBody
	{
	public:
		EditorBody(UAGX_RigidBodyComponent& InBody, FAGX_ArchiveImporter& InHelper)
			: Helper(InHelper)
			, Body(InBody)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
		{
			Helper.InstantiateSphere(Barrier, Body);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
		{
			Helper.InstantiateBox(Barrier, Body);
		}

		virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
		{
			Helper.InstantiateCylinder(Barrier, Body);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
		{
			Helper.InstantiateTrimesh(Barrier, Body);
		}

	private:
		FAGX_ArchiveImporter& Helper;
		UAGX_RigidBodyComponent& Body;
	};

	class EditorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		EditorInstantiator(AActor& InImportedRoot, UWorld& InWorld, FAGX_ArchiveImporter& InHelper)
			: Helper(InHelper)
			, ImportedRoot(InImportedRoot)
			, World(InWorld)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			AAGX_RigidBodyActor* NewActor = Helper.InstantiateBody(Barrier, World);
			if (NewActor == nullptr)
			{
				return new NopEditorBody();
			}
			NewActor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			return new EditorBody(*NewActor->RigidBodyComponent, Helper);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Hinge) override
		{
			CreateConstraint1Dof(Hinge, AAGX_HingeConstraintActor::StaticClass());
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Prismatic) override
		{
			CreateConstraint1Dof(Prismatic, AAGX_PrismaticConstraintActor::StaticClass());
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& BallJoint) override
		{
			CreateConstraint<AAGX_BallConstraintActor>(
				BallJoint, AAGX_BallConstraintActor::StaticClass());
		}

		virtual void InstantiateCylindricalJoint(
			const FCylindricalJointBarrier& CylindricalJoint) override
		{
			CreateConstraint2Dof(CylindricalJoint, AAGX_CylindricalConstraintActor::StaticClass());
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& DistanceJoint) override
		{
			CreateConstraint1Dof(DistanceJoint, AAGX_DistanceConstraintActor::StaticClass());
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& LockJoint) override
		{
			CreateConstraint<AAGX_LockConstraintActor>(
				LockJoint, AAGX_LockConstraintActor::StaticClass());
		}

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledGroups) override
		{
			if (DisabledGroups.Num() == 0)
			{
				// Do not force a CollisionGroupManager if there are no disabled groups.
				return;
			}

			/// \todo Consider creating a GetOrCreateFrom member function to CollisionGroupManager.
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

		virtual void InstantiateShapeMaterial(
			const FShapeMaterialBarrier& Barrier) override
		{
			Helper.InstantiateShapeMaterial(Barrier);
		}

		virtual void InstantiateContactMaterial(
				const FContactMaterialBarrier& Barrier) override
		{
			Helper.InstantiateContactMaterial(Barrier);
		}

		virtual ~EditorInstantiator() = default;

	private:
		void CreateConstraint1Dof(const FConstraint1DOFBarrier& Barrier, UClass* ConstraintType)
		{
			AAGX_Constraint1DofActor* Constraint =
				CreateConstraint<AAGX_Constraint1DofActor>(Barrier, ConstraintType);
			if (Constraint == nullptr)
			{
				// No need to log here, done by CreateConstraint.
				return;
			}

			UAGX_Constraint1DofComponent* Component = Constraint->Get1DofComponent();
			StoreElectricMotorController(Barrier, Component->ElectricMotorController);
			StoreFrictionController(Barrier, Component->FrictionController);
			StoreLockController(Barrier, Component->LockController);
			StoreRangeController(Barrier, Component->RangeController);
			StoreTargetSpeedController(Barrier, Component->TargetSpeedController);
		}

		void CreateConstraint2Dof(const FConstraint2DOFBarrier& Barrier, UClass* ConstraintType)
		{
			AAGX_Constraint2DofActor* Constraint =
				CreateConstraint<AAGX_Constraint2DofActor>(Barrier, ConstraintType);
			if (Constraint == nullptr)
			{
				// No need to log here, done by CreateConstraint.
				return;
			}

			UAGX_Constraint2DofComponent* Component = Constraint->Get2DofComponent();

			const EAGX_Constraint2DOFFreeDOF First = EAGX_Constraint2DOFFreeDOF::FIRST;
			const EAGX_Constraint2DOFFreeDOF Second = EAGX_Constraint2DOFFreeDOF::SECOND;

			StoreElectricMotorController(Barrier, Component->ElectricMotorController1, First);
			StoreElectricMotorController(Barrier, Component->ElectricMotorController2, Second);

			StoreFrictionController(Barrier, Component->FrictionController1, First);
			StoreFrictionController(Barrier, Component->FrictionController2, Second);

			StoreLockController(Barrier, Component->LockController1, First);
			StoreLockController(Barrier, Component->LockController2, Second);

			StoreRangeController(Barrier, Component->RangeController1, First);
			StoreRangeController(Barrier, Component->RangeController2, Second);

			StoreTargetSpeedController(Barrier, Component->TargetSpeedController1, First);
			StoreTargetSpeedController(Barrier, Component->TargetSpeedController2, Second);
		}

		template <typename ConstraintType>
		ConstraintType* CreateConstraint(const FConstraintBarrier& Barrier, UClass* ConstraintClass)
		{
			std::pair<AAGX_RigidBodyActor*, AAGX_RigidBodyActor*> Actors = GetActors(Barrier);
			if (Actors.first == nullptr)
			{
				// Not having a second body is fine. Means that the first body
				// is constrained to the world. Not having a first body is bad.
				UE_LOG(
					LogAGX, Log, TEXT("Constraint %s doesn't have a first body. Ignoring."),
					*Barrier.GetName());
				return nullptr;
			}

			ConstraintType* Constraint =
				FAGX_EditorUtilities::CreateConstraintActor<ConstraintType>(
					Actors.first->RigidBodyComponent, Actors.second->RigidBodyComponent,
					/*bSelect*/ false, /*bShwNotification*/ false, /*bInPlayingWorld*/ false,
					ConstraintClass);

			StoreFrames(Barrier, *Constraint->GetConstraintComponent());

			/// \todo Is there a correct transform here? Does it matter?
			Constraint->SetActorTransform(Actors.first->GetActorTransform());
			Constraint->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			if (!Barrier.GetName().IsEmpty())
			{
				FString Name = Barrier.GetName();
				if (!Constraint->Rename(*Name, nullptr, REN_Test))
				{
					FString OldName = Name;
					Name = MakeUniqueObjectName(
							   Constraint->GetOuter(), ConstraintType::StaticClass(), FName(*Name))
							   .ToString();
					UE_LOG(
						LogAGX, Warning,
						TEXT("Constraint '%s' imported with name '%s' because of name collision."),
						*OldName, *Name);
				}
				/// \todo I think it's enough to do SetActorLabel here.
				/// It matters if we are in Editor mode or not.
				Constraint->Rename(*Name);
				Constraint->SetActorLabel(*Name);
			}

			return Constraint;
		}

		AAGX_RigidBodyActor* GetActor(const FRigidBodyBarrier& Barrier)
		{
#if 1
			UAGX_RigidBodyComponent* Component = Helper.GetBody(Barrier);
			if (Component == nullptr)
			{
				/// \todo Print warning here? What should the warning say?
				return nullptr;
			}
			/// \todo Print warning if we find a Component that isn't owned by a RigidBodyActor?
			return Cast<AAGX_RigidBodyActor>(Component->GetOwner());
#else
			if (!Body.HasNative())
			{
				// No log since not an error. Means constrained with world.
				return nullptr;
			}
			FGuid Guid = Body.GetGuid();
			AAGX_RigidBodyActor* Actor = Bodies.FindRef(Guid);
			if (Actor == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found a constraint to body '%s', but that body isn't known."),
					*Body.GetName());
				return nullptr;
			}
			return Actor;
#endif
		}

		std::pair<AAGX_RigidBodyActor*, AAGX_RigidBodyActor*> GetActors(
			const FConstraintBarrier& Barrier)
		{
			return {GetActor(Barrier.GetFirstBody()), GetActor(Barrier.GetSecondBody())};
		}

		void StoreFrame(
			const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment,
			int32 BodyIndex)
		{
			Attachment.FrameDefiningComponent.Clear();
			Attachment.LocalFrameLocation = Barrier.GetLocalLocation(BodyIndex);
			Attachment.LocalFrameRotation = Barrier.GetLocalRotation(BodyIndex);
		}

		void StoreFrames(const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Constraint)
		{
			StoreFrame(Barrier, Constraint.BodyAttachment1, 0);
			StoreFrame(Barrier, Constraint.BodyAttachment2, 1);
		}

		void StoreElectricMotorController(
			const FConstraint1DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetElectricMotorController());
		}

		void StoreElectricMotorController(
			const FConstraint2DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller, EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetElectricMotorController(Dof));
		}

		void StoreFrictionController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetFrictionController());
		}

		void StoreFrictionController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetFrictionController(Dof));
		}

		void StoreLockController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetLockController());
		}

		void StoreLockController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetLockController(Dof));
		}

		void StoreRangeController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetRangeController());
		}

		void StoreRangeController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetRangeController(Dof));
		}

		void StoreTargetSpeedController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetTargetSpeedController());
		}

		void StoreTargetSpeedController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetTargetSpeedController(Dof));
		}

	private:
		FAGX_ArchiveImporter Helper;
		AActor& ImportedRoot;
		UWorld& World;
	};

}

AActor* AGX_ArchiveImporterToActorTree::ImportAGXArchive(const FString& ArchivePath)
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

	FString Filename = FPaths::GetBaseFilename(ArchivePath);
	ImportGroup->SetActorLabel(Filename);

	FAGX_ArchiveImporter Helper(ArchivePath);
	EditorInstantiator Instantiator(*ImportGroup, *World, Helper);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE
