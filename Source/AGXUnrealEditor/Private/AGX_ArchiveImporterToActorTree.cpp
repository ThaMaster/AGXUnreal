#include "AGX_ArchiveImporterToActorTree.h"

// AGXUnreal includes.
#include "AGX_ArchiveImporterHelper.h"
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
		EditorBody(UAGX_RigidBodyComponent& InBody, FAGX_ArchiveImporterHelper& InHelper)
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
		FAGX_ArchiveImporterHelper& Helper;
		UAGX_RigidBodyComponent& Body;
	};

	class EditorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		EditorInstantiator(
			AActor& InImportedRoot, UWorld& InWorld, FAGX_ArchiveImporterHelper& InHelper)
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

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			AAGX_HingeConstraintActor* Actor = Helper.InstantiateHinge(Barrier);
			FinalizeConstraint(Actor);
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			AAGX_PrismaticConstraintActor* Actor = Helper.InstantiatePrismatic(Barrier);
			FinalizeConstraint(Actor);
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& Barrier) override
		{
			AAGX_BallConstraintActor* Actor = Helper.InstantiateBallJoint(Barrier);
			FinalizeConstraint(Actor);
		}

		virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& Barrier) override
		{
			AAGX_CylindricalConstraintActor* Actor = Helper.InstantiateCylindricalJoint(Barrier);
			FinalizeConstraint(Actor);
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& Barrier) override
		{
			AAGX_DistanceConstraintActor* Actor = Helper.InstantiateDistanceJoint(Barrier);
			FinalizeConstraint(Actor);
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& Barrier) override
		{
			AAGX_LockConstraintActor* Actor = Helper.InstantiateLockJoint(Barrier);
			FinalizeConstraint(Actor);
		}

		void FinalizeConstraint(AAGX_ConstraintActor* Actor)
		{
			if (Actor == nullptr)
			{
				return;
			}
			Actor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
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

		virtual void InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier) override
		{
			Helper.InstantiateShapeMaterial(Barrier);
		}

		virtual void InstantiateContactMaterial(const FContactMaterialBarrier& Barrier) override
		{
			Helper.InstantiateContactMaterial(Barrier);
		}

		virtual ~EditorInstantiator() = default;

	private:
		FAGX_ArchiveImporterHelper Helper;
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

	FAGX_ArchiveImporterHelper Helper(ArchivePath);
	EditorInstantiator Instantiator(*ImportGroup, *World, Helper);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE
