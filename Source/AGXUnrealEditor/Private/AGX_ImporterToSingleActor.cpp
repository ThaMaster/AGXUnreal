#include "AGX_ImporterToSingleActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ArchiveImporterHelper.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGXSimObjectsReader.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Math/Transform.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	class FSingleActorBody final : public FAGXSimObjectBody
	{
	public:
		FSingleActorBody(UAGX_RigidBodyComponent& InBody, FAGX_ArchiveImporterHelper& InHelper)
			: Body(InBody)
			, Helper(InHelper)
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
		UAGX_RigidBodyComponent& Body;
		FAGX_ArchiveImporterHelper& Helper;
	};

	/**
	 * An archive instantiator that creates all objects as Components in a given Actor.
	 */
	class SingleActorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		SingleActorInstantiator(
			UWorld& InWorld, AActor& InActor, USceneComponent& InRoot,
			const FString& InArchiveFilePath)
			: Helper(InArchiveFilePath)
			, World(InWorld)
			, Actor(InActor)
			, Root(InRoot)
		{
		}

		virtual FAGXSimObjectBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component = Helper.InstantiateBody(Barrier, Actor);
			if (Component == nullptr)
			{
				return new NopEditorBody();
			}
			return new FSingleActorBody(*Component, Helper);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			Helper.InstantiateHinge(Barrier, Actor);
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			Helper.InstantiatePrismatic(Barrier, Actor);
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& Barrier) override
		{
			Helper.InstantiateBallJoint(Barrier, Actor);
		}

		virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& Barrier) override
		{
			Helper.InstantiateCylindricalJoint(Barrier, Actor);
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& Barrier) override
		{
			Helper.InstantiateDistanceJoint(Barrier, Actor);
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& Barrier) override
		{
			Helper.InstantiateLockJoint(Barrier, Actor);
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
				Helper.InstantiateSphere(Barrier, Actor);
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
				Helper.InstantiateBox(Barrier, Actor);
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
				Helper.InstantiateCylinder(Barrier, Actor);
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
				Helper.InstantiateCapsule(Barrier, Actor);
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
				Helper.InstantiateTrimesh(Barrier, Actor);
			}
		}
		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledPairs) override
		{
			if (DisabledPairs.Num() == 0)
			{
				return;
			}

			Helper.InstantiateCollisionGroupDisabler(Actor, DisabledPairs);
		}

		virtual void InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier) override
		{
			Helper.InstantiateShapeMaterial(Barrier);
		}

		virtual void InstantiateContactMaterial(const FContactMaterialBarrier& Barrier) override
		{
			Helper.InstantiateContactMaterial(Barrier);
		}

		virtual FTwoBodyTireArchiveBodies InstantiateTwoBodyTire(
			const FTwoBodyTireBarrier& Barrier) override
		{
			// Instantiate the Tire and Hub Rigid Bodies. This adds them to the RestoredBodies TMap
			// and can thus be found and used when the TwoBodyTire component is instantiated.
			const FRigidBodyBarrier TireBody = Barrier.GetTireRigidBody();
			const FRigidBodyBarrier HubBody = Barrier.GetHubRigidBody();
			if (TireBody.HasNative() == false || HubBody.HasNative() == false)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("At lest one of the Rigid Bodies referenced by the TwoBodyTire %s did not "
						 "have a native Rigid Body. The TwoBodyTire will not be instantiated."),
					*Barrier.GetName());
				return FTwoBodyTireArchiveBodies(new NopEditorBody(), new NopEditorBody());
			}

			FTwoBodyTireArchiveBodies ArchiveBodies;
			ArchiveBodies.TireBodyArchive.reset(InstantiateBody(TireBody));
			ArchiveBodies.HubBodyArchive.reset(InstantiateBody(HubBody));

			Helper.InstantiateTwoBodyTire(Barrier, Actor);
			return ArchiveBodies;
		}

		virtual void InstantiateWire(const FWireBarrier& Barrier) override
		{
			Helper.InstantiateWire(Barrier, Actor);
		}

	private:
		FAGX_ArchiveImporterHelper Helper;
		UWorld& World;
		AActor& Actor;
		USceneComponent& Root;
	};
}

AActor* AGX_ImporterToSingleActor::ImportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("No world available cannot import AGX Dynamics archive."));
		return nullptr;
	}

	AActor* NewActor;
	USceneComponent* NewRoot;
	std::tie(NewActor, NewRoot) =
		FAGX_EditorUtilities::CreateEmptyActor(FTransform::Identity, World);
	if (NewActor == nullptr || NewRoot == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot import AGX Dynamics archive because new actors cannot be created."));
		return nullptr;
	}

	FString Filename = FPaths::GetBaseFilename(ArchivePath);
	NewActor->SetActorLabel(Filename);

	SingleActorInstantiator Instantiator(*World, *NewActor, *NewRoot, ArchivePath);
	FAGXSimObjectsReader::Read(ArchivePath, Instantiator);
	return NewActor;
}
