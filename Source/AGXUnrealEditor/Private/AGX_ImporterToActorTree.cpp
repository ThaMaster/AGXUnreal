#include "AGX_ImporterToActorTree.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SimObjectsImporterHelper.h"
#include "AGXSimObjectsReader.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"
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
#include "Constraints/BallJointBarrier.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Tires/AGX_TwoBodyTireActor.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Wire/AGX_WireComponent.h"

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
	// Simulation objects instantiator that creates sub-objects under RigidBody. Knows how
	// to create various subclasses of AGX_ShapesComponent in Unreal Editor.
	class FEditorBody final : public FAGXSimObjectBody
	{
	public:
		FEditorBody(UAGX_RigidBodyComponent& InBody, FAGX_SimObjectsImporterHelper& InHelper)
			: Helper(InHelper)
			, Body(InBody)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
		{
			Helper.InstantiateSphere(Barrier, *Body.GetOwner(), &Body);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
		{
			Helper.InstantiateBox(Barrier, *Body.GetOwner(), &Body);
		}

		virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
		{
			Helper.InstantiateCylinder(Barrier, *Body.GetOwner(), &Body);
		}

		virtual void InstantiateCapsule(const FCapsuleShapeBarrier& Barrier) override
		{
			Helper.InstantiateCapsule(Barrier, *Body.GetOwner(), &Body);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
		{
			Helper.InstantiateTrimesh(Barrier, *Body.GetOwner(), &Body);
		}

	private:
		FAGX_SimObjectsImporterHelper& Helper;
		UAGX_RigidBodyComponent& Body;
	};

	class EditorInstantiator final : public FAGXSimObjectsInstantiator
	{
	public:
		EditorInstantiator(
			AActor& InImportedRoot, UWorld& InWorld, FAGX_SimObjectsImporterHelper& InHelper)
			: Helper(InHelper)
			, ImportedRoot(InImportedRoot)
			, World(InWorld)
		{
		}

		virtual FAGXSimObjectBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			AAGX_RigidBodyActor* NewActor = Helper.InstantiateBody(Barrier, World);
			if (NewActor == nullptr)
			{
				return new NopEditorBody();
			}
			NewActor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			return new FEditorBody(*NewActor->RigidBodyComponent, Helper);
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

		virtual void InstantiateSphere(
			const FSphereShapeBarrier& Barrier, FAGXSimObjectBody* Body) override
		{
			if (Body != nullptr)
			{
				Body->InstantiateSphere(Barrier);
			}
			else
			{
				Helper.InstantiateSphere(Barrier, ImportedRoot);
			}
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier, FAGXSimObjectBody* Body) override
		{
			if (Body != nullptr)
			{
				Body->InstantiateBox(Barrier);
			}
			else
			{
				Helper.InstantiateBox(Barrier, ImportedRoot);
			}
		}

		virtual void InstantiateCylinder(
			const FCylinderShapeBarrier& Barrier, FAGXSimObjectBody* Body) override
		{
			if (Body != nullptr)
			{
				Body->InstantiateCylinder(Barrier);
			}
			else
			{
				Helper.InstantiateCylinder(Barrier, ImportedRoot);
			}
		}

		virtual void InstantiateCapsule(
			const FCapsuleShapeBarrier& Barrier, FAGXSimObjectBody* Body) override
		{
			if (Body != nullptr)
			{
				Body->InstantiateCapsule(Barrier);
			}
			else
			{
				Helper.InstantiateCapsule(Barrier, ImportedRoot);
			}
		}

		virtual void InstantiateTrimesh(
			const FTrimeshShapeBarrier& Barrier, FAGXSimObjectBody* Body) override
		{
			if (Body != nullptr)
			{
				Body->InstantiateTrimesh(Barrier);
			}
			else
			{
				Helper.InstantiateTrimesh(Barrier, ImportedRoot);
			}
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
				return;
			}

			AAGX_CollisionGroupDisablerActor* Actor =
				Helper.InstantiateCollisionGroupDisabler(World, DisabledGroups);
			if (Actor)
			{
				Actor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
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

		virtual FTwoBodyTireSimObjectBodies InstantiateTwoBodyTire(
			const FTwoBodyTireBarrier& Barrier) override
		{
			AAGX_TwoBodyTireActor* Actor = Helper.InstantiateTwoBodyTire(Barrier, World);
			if (Actor == nullptr)
			{
				return FTwoBodyTireSimObjectBodies(new NopEditorBody, new NopEditorBody);
			}

			Actor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			return FTwoBodyTireSimObjectBodies(
				new FEditorBody(*Actor->TireRigidBodyComponent, Helper),
				new FEditorBody(*Actor->HubRigidBodyComponent, Helper));
		}

		virtual void InstantiateWire(const FWireBarrier& Barrier) override
		{
			AActor* Actor = Helper.InstantiateWire(Barrier, World);
			Actor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
		}

		virtual ~EditorInstantiator() = default;

	private:
		FAGX_SimObjectsImporterHelper Helper;
		AActor& ImportedRoot;
		UWorld& World;
	};

}

AActor* AGX_ImporterToActorTree::ImportAGXArchive(const FString& ArchivePath)
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

	FAGX_SimObjectsImporterHelper Helper(ArchivePath);
	EditorInstantiator Instantiator(*ImportGroup, *World, Helper);
	FAGXSimObjectsReader::ReadAGXArchive(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE
