#include "AGX_ArchiveImporterToSingleActor.h"

// AGXUnreal includes.
#include "AGX_ArchiveImporterHelper.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGXArchiveReader.h"
#include "CollisionGroups/AGX_CollisionGroupManager.h"
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
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Math/Transform.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	class SingleActorBody final : public FAGXArchiveBody
	{
	public:
		SingleActorBody(UAGX_RigidBodyComponent& InBody, FAGX_ArchiveImporterHelper& InHelper)
			: Body(InBody)
			, Helper(InHelper)
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

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component = Helper.InstantiateBody(Barrier, Actor);
			if (Component == nullptr)
			{
				return new NopEditorBody();
			}
			return new SingleActorBody(*Component, Helper);
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

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledPairs) override
		{
			if (DisabledPairs.Num() == 0)
			{
				return;
			}

			AAGX_CollisionGroupManager* Manager = AAGX_CollisionGroupManager::GetFrom(&World);
			if (Manager == nullptr)
			{
				Manager = World.SpawnActor<AAGX_CollisionGroupManager>();
			}
			if (Manager == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Cannot import disabled collision group pairs because there is no "
						 "CollisionGroupManager in the level and one could not be created."));
				return;
			}

			for (auto& Pair : DisabledPairs)
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

		virtual void InstantiateTwoBodyTire(const FTwoBodyTireBarrier& Barrier) override
		{
			 Helper.InstantiateTwoBodyTire(Barrier, Actor);
		}

	private:
		FAGX_ArchiveImporterHelper Helper;
		UWorld& World;
		AActor& Actor;
		USceneComponent& Root;
	};
}

AActor* AGX_ArchiveImporterToSingleActor::ImportAGXArchive(const FString& ArchivePath)
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
	FAGXArchiveReader::Read(ArchivePath, Instantiator);
	return NewActor;
}
