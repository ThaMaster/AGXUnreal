#include "AGX_ArchiveImporterToSingleActor.h"

// AGXUnreal includes.
#include "AGXArchiveReader.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
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
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Math/Transform.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Map.h"

namespace
{
	class SingleActorBody final : public FAGXArchiveBody
	{
	public:
		SingleActorBody(UAGX_RigidBodyComponent& InBody, const FString& InArchiveName)
			: Body(InBody)
			, ArchiveName(InArchiveName)
		{
		}

		virtual void InstantiateSphere(
			const FSphereShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
			UAGX_SphereShapeComponent* Component =
				FAGX_EditorUtilities::CreateSphereShape(Body.GetOwner(), &Body);
			Component->Radius = Barrier.GetRadius();
			FinalizeShape(*Component, Barrier, ShapeMaterialAsset);
		}

		virtual void InstantiateBox(
			const FBoxShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
			UAGX_BoxShapeComponent* Component =
				FAGX_EditorUtilities::CreateBoxShape(Body.GetOwner(), &Body);
			Component->HalfExtent = Barrier.GetHalfExtents();
			FinalizeShape(*Component, Barrier, ShapeMaterialAsset);
		}

		virtual void InstantiateCylinder(
			const FCylinderShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
			UAGX_CylinderShapeComponent* Component =
				FAGX_EditorUtilities::CreateCylinderShape(Body.GetOwner(), &Body);
			Component->Height = Barrier.GetHeight();
			Component->Radius = Barrier.GetRadius();
			FinalizeShape(*Component, Barrier, ShapeMaterialAsset);
		}

		virtual void InstantiateTrimesh(
			const FTrimeshShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
			AActor* Owner = Body.GetOwner();
			UAGX_TrimeshShapeComponent* Component =
				FAGX_EditorUtilities::CreateTrimeshShape(Owner, &Body);
			Component->MeshSourceLocation =
				EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
			UStaticMeshComponent* MeshComponent =
				FAGX_EditorUtilities::CreateStaticMeshAsset(Owner, Component, Barrier, ArchiveName);

			FString Name = MeshComponent->GetName() + "Shape";
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(Owner, UAGX_TrimeshShapeComponent::StaticClass(), *Name)
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Trimesh '%s' imported with name '%s' vecause of name conflict."),
					*OldName, *Name);
			}
			Component->Rename(*Name, nullptr, REN_DontCreateRedirectors);
			FinalizeShape(*Component, Barrier, ShapeMaterialAsset);
		}

	private:
		void FinalizeShape(
			UAGX_ShapeComponent& Component, const FShapeBarrier& Barrier,
			const FString& ShapeMaterialAsset)
		{
			FAGX_EditorUtilities::ApplyShapeMaterial(&Component, ShapeMaterialAsset);

			Component.SetFlags(RF_Transactional);
			Component.bCanCollide = Barrier.GetEnableCollisions();
			for (const FName& Group : Barrier.GetCollisionGroups())
			{
				Component.AddCollisionGroup(Group);
			}
			Component.SetRelativeLocation(Barrier.GetLocalPosition());
			Component.SetRelativeRotation(Barrier.GetLocalRotation());
			Component.UpdateVisualMesh();
			FString Name = Barrier.GetName();
			/// \todo This pattern is repeated in multiple places. Find a way to extract.
			if (!Component.Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(Component.GetOwner(), Component.GetClass(), *Name)
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Shape '%s' imported with name '%s' because of name conflict."), *OldName,
					*Name);
			}
			Component.Rename(*Name, nullptr, REN_DontCreateRedirectors);
		}

	private:
		UAGX_RigidBodyComponent& Body;
		const FString& ArchiveName;
	};

	/// \todo Consider moving this to the generic importer file. It's the same for all importers.
	/**
	 * An ArchiveBody that creates nothing. Used when the Unreal object couldn't be created.
	 */
	class NopEditorBody final : public FAGXArchiveBody
	{
		virtual void InstantiateSphere(
			const FSphereShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
		}

		virtual void InstantiateBox(
			const FBoxShapeBarrier& Box, const FString& ShapeMaterialAsset) override
		{
		}

		virtual void InstantiateCylinder(
			const FCylinderShapeBarrier& Cylinder, const FString& ShapeMaterialAsset) override
		{
		}

		virtual void InstantiateTrimesh(
			const FTrimeshShapeBarrier& Barrier, const FString& ShapeMaterialAsset) override
		{
		}
	};

	/**
	 * An archive instantiator that creates all objects as Components in a given Actor.
	 */
	class SingleActorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		SingleActorInstantiator(
			UWorld& InWorld, AActor& InActor, USceneComponent& InRoot, const FString& InArchiveName)
			: World(InWorld)
			, Actor(InActor)
			, Root(InRoot)
			, ArchiveName(InArchiveName)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component =
				NewObject<UAGX_RigidBodyComponent>(&Actor, NAME_None);
			if (Component == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Could not import AGX Dynamics body '%s'. Could not create a new "
						 "RigidBodyComponent."),
					*Barrier.GetName());
				return new NopEditorBody();
			}

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(&Actor, UAGX_RigidBodyComponent::StaticClass(), *Name)
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("RigidBody '%s' imported with name '%s' because of name conflict"),
					*OldName, *Name);
			}
			Component->Rename(*Name);

			Component->SetWorldLocation(Barrier.GetPosition());
			Component->SetWorldRotation(Barrier.GetRotation());
			Component->Mass = Barrier.GetMass();
			Component->MotionControl = Barrier.GetMotionControl();

			Component->SetFlags(RF_Transactional);
			Actor.AddInstanceComponent(Component);
			Component->RegisterComponent();

			Component->AttachToComponent(&Root, FAttachmentTransformRules::KeepWorldTransform);

			Component->PostEditChange();
			RestoredBodies.Add(Barrier.GetGuid(), Component);
			return new SingleActorBody(*Component, ArchiveName);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_HingeConstraintComponent::StaticClass());
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_PrismaticConstraintComponent::StaticClass());
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& Barrier) override
		{
			InstantiateConstraint<UAGX_ConstraintComponent>(
				Barrier, UAGX_BallConstraintComponent::StaticClass());
		}

		virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& Barrier) override
		{
			InstantiateConstraint2Dof(Barrier, UAGX_CylindricalConstraintComponent::StaticClass());
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_DistanceConstraintComponent::StaticClass());
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& Barrier) override
		{
			InstantiateConstraint<UAGX_ConstraintComponent>(
				Barrier, UAGX_LockConstraintComponent::StaticClass());
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

		virtual FString CreateMaterialAsset(const FShapeMaterialBarrier& ShapeMaterial) override
		{
			FString AssetPath =
				FAGX_EditorUtilities::CreateShapeMaterialAsset(ArchiveName, ShapeMaterial);

			return AssetPath;
		}

	private:
		using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;

	private:
		void InstantiateConstraint1Dof(const FConstraint1DOFBarrier& Barrier, UClass* Type)
		{
			UAGX_Constraint1DofComponent* Component =
				InstantiateConstraint<UAGX_Constraint1DofComponent>(Barrier, Type);
			if (Component == nullptr)
			{
				// No need to log here, done by InstantiateConstraint.
				return;
			}

			FAGX_ConstraintUtilities::StoreControllers(*Component, Barrier);
		}

		void InstantiateConstraint2Dof(const FConstraint2DOFBarrier& Barrier, UClass* Type)
		{
			UAGX_Constraint2DofComponent* Component =
				InstantiateConstraint<UAGX_Constraint2DofComponent>(Barrier, Type);
			if (Component == nullptr)
			{
				// No need to log here, done by InstantiateConstraint.
				return;
			}

			FAGX_ConstraintUtilities::StoreControllers(*Component, Barrier);
		}

		template <typename UConstraint>
		UConstraint* InstantiateConstraint(const FConstraintBarrier& Barrier, UClass* Type)
		{
			FBodyPair Bodies = GetBodies(Barrier);
			if (Bodies.first == nullptr)
			{
				UE_LOG(
					LogAGX, Warning, TEXT("Constraint '%s' does not have a first body. Ignoring."),
					*Barrier.GetName());
				return nullptr;
			}

			UConstraint* Component = FAGX_EditorUtilities::CreateConstraintComponent<UConstraint>(
				&Actor, Bodies.first, Bodies.second, Type);
			if (Component == nullptr)
			{
				return nullptr;
			}

			FAGX_ConstraintUtilities::StoreFrames(Barrier, *Component);

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(&Actor, UConstraint::StaticClass(), *Name).ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Constraint '%s' imported with name '%s' because of name collision."),
					*OldName, *Name);
			}
			Component->Rename(*Name, nullptr, REN_DontCreateRedirectors);

			return Component;
		}

		UAGX_RigidBodyComponent* GetBody(const FRigidBodyBarrier& Barrier)
		{
			if (!Barrier.HasNative())
			{
				// Not an error, means constrainted with world.
				return nullptr;
			}
			FGuid Guid = Barrier.GetGuid();
			UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Guid);
			if (Component == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found a constraint to body '%s', but that body hans't been restored."),
					*Barrier.GetName());
				return nullptr;
			}
			return Component;
		}

		FBodyPair GetBodies(const FConstraintBarrier& Constraint)
		{
			return {GetBody(Constraint.GetFirstBody()), GetBody(Constraint.GetSecondBody())};
		}

	private:
		UWorld& World;
		AActor& Actor;
		USceneComponent& Root;
		const FString& ArchiveName;
		TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
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

	SingleActorInstantiator Instantiator(*World, *NewActor, *NewRoot, Filename);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);
	return NewActor;
}
