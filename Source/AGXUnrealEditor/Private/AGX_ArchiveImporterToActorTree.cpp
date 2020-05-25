#include "AGX_ArchiveImporterToActorTree.h"

// AGXUnreal includes.
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
	AAGX_RigidBodyActor* InstantiateBody(const FRigidBodyBarrier& Body, UWorld& World)
	{
		FTransform Transform(Body.GetRotation(), Body.GetPosition());
		UE_LOG(LogAGX, Log, TEXT("Loaded AGX body with name '%s'."), *Body.GetName());
		AAGX_RigidBodyActor* NewActor =
			World.SpawnActor<AAGX_RigidBodyActor>(AAGX_RigidBodyActor::StaticClass(), Transform);
		if (NewActor == nullptr)
		{
			UE_LOG(LogAGX, Log, TEXT("Could not create Actor for body '%s'."), *Body.GetName());
			return nullptr;
		}
		NewActor->SetActorLabel(Body.GetName()); /// \todo Rename instead of SetActorLabel?
		UAGX_RigidBodyComponent* NewBody = NewActor->RigidBodyComponent;

		/// \todo Move property setting to a more central location, perhaps
		/// in a member function in UAGX_RigidBodyComponent. Decide on a pattern
		/// to be used for all classes backed by a Barrier.
		NewBody->Mass = Body.GetMass();
		NewBody->MotionControl = Body.GetMotionControl();

		// It is tempting to rename the body here, i.e., NewBody->Rename, but
		// that breaks things. I suspect the reason is that because we create
		// the RigidBodyComponent with CreateDefaultSubobject in C++, Unreal
		// Engine assumes that such a component should be there. When we do the
		// rename here then Unreal Engine doesn't find it and creates a new one.
		// Badness ensues.

		return NewActor;
	}

	// Archive instantiator that creates sub-objects under RigidBody. Knows how
	// to create various subclasses of AGX_ShapesComponent in Unreal Editor.
	class EditorBody final : public FAGXArchiveBody
	{
	public:
		EditorBody(
			AActor& InActor, USceneComponent& InRoot, const FString& InArchiveName,
			TMap<FGuid, UStaticMesh*>* InMeshAssets)
			: Actor(InActor)
			, Root(InRoot)
			, ArchiveName(InArchiveName)
			, MeshAssets(InMeshAssets)
		{
		}

		virtual void InstantiateSphere(
			const FSphereShapeBarrier& Sphere, const FString& ShapeMaterialAsset) override
		{
			UAGX_SphereShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateSphereShape(&Actor, &Root);
			ShapeComponent->Radius = Sphere.GetRadius();
			FinalizeShape(ShapeComponent, Sphere, ShapeMaterialAsset);
		}

		virtual void InstantiateBox(
			const FBoxShapeBarrier& Box, const FString& ShapeMaterialAsset) override
		{
			UAGX_BoxShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateBoxShape(&Actor, &Root);
			ShapeComponent->HalfExtent = Box.GetHalfExtents();
			FinalizeShape(ShapeComponent, Box, ShapeMaterialAsset);
		}

		virtual void InstantiateCylinder(
			const FCylinderShapeBarrier& Cylinder, const FString& ShapeMaterialAsset) override
		{
			UAGX_CylinderShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateCylinderShape(&Actor, &Root);
			ShapeComponent->Radius = Cylinder.GetRadius();
			ShapeComponent->Height = Cylinder.GetHeight();
			FinalizeShape(ShapeComponent, Cylinder, ShapeMaterialAsset);
		}

		virtual void InstantiateTrimesh(
			const FTrimeshShapeBarrier& Trimesh, const FString& ShapeMaterialAsset) override
		{
			UAGX_TrimeshShapeComponent* ShapeComponent =
				FAGX_EditorUtilities::CreateTrimeshShape(&Actor, &Root);
			ShapeComponent->MeshSourceLocation =
				EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
			UStaticMesh* MeshAsset = GetOrCreateStaticMeshAsset(&Actor, Trimesh);
			if (!MeshAsset)
			{
				// No point in continuing further. Logging handled in GetOrCreateStaticMeshAsset.
				return;
			}

			UStaticMeshComponent* MeshComponent =
				FAGX_EditorUtilities::CreateStaticMeshComponent(&Actor, ShapeComponent, MeshAsset);

			FString Name = MeshComponent->GetName() + "Shape";
			if (!ShapeComponent->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(
						   &Actor, UAGX_TrimeshShapeComponent::StaticClass(), FName(*Name))
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Trimesh '%s' imported with name '%s' because of name conflict."),
					*OldName, *Name);
			}
			ShapeComponent->Rename(*Name, nullptr, REN_DontCreateRedirectors);
			FinalizeShape(ShapeComponent, Trimesh, ShapeMaterialAsset);
		}

	private:
		void FinalizeShape(
			UAGX_ShapeComponent* Component, const FShapeBarrier& Barrier,
			const FString& ShapeMaterialAsset)
		{
			if (!ShapeMaterialAsset.IsEmpty())
			{
				const bool Result =
					FAGX_EditorUtilities::ApplyShapeMaterial(Component, ShapeMaterialAsset);
				if (!Result)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("ApplyShapeMaterial in FinalizeShape failed. Actor: %s, Shape "
							 "Component: %s, Asset: %s."),
						*Actor.GetActorLabel(), *Component->GetName(), *ShapeMaterialAsset);
				}
			}

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

			/// \todo Rename the shape.
		}

		UStaticMesh* GetOrCreateStaticMeshAsset(AActor* Owner, const FTrimeshShapeBarrier& Barrier)
		{
			FGuid Guid = Barrier.GetMeshDataGuid();
			if (!Guid.IsValid())
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Unable to create static mesh asset from TrimeshShapeBarrier: %s for "
						 "owner: %s since the TrimeshShapeBarrier did not have a valid Guid."),
					*Barrier.GetSourceName(), *Owner->GetName());

				return nullptr;
			}

			if (MeshAssets->Find(Guid))
			{
				return (*MeshAssets)[Guid];
			}

			UStaticMesh* MeshAsset =
				FAGX_EditorUtilities::CreateStaticMeshAsset(Barrier, ArchiveName);
			MeshAssets->Add(Guid, MeshAsset);

			return MeshAsset;
		}

		AActor& Actor;
		USceneComponent& Root;
		const FString& ArchiveName;
		TMap<FGuid, UStaticMesh*>* MeshAssets;
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
			AAGX_RigidBodyActor* NewActor = ::InstantiateBody(Barrier, World);
			if (NewActor == nullptr)
			{
				return nullptr;
			}
			NewActor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			Bodies.Add(Barrier.GetGuid(), NewActor);
			return new EditorBody(
				*NewActor, *NewActor->RigidBodyComponent, ImportedRoot.GetActorLabel(), &MeshAssets);
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

		virtual FString CreateShapeMaterialAsset(const FShapeMaterialBarrier& ShapeMaterial) override
		{
			return FAGX_EditorUtilities::CreateShapeMaterialAsset(
				ImportedRoot.GetActorLabel(), ShapeMaterial);
		}

		virtual FString CreateContactMaterialAsset(
			const FContactMaterialBarrier& ContactMaterial, const FString& Material1,
			const FString& Material2) override
		{
			return FAGX_EditorUtilities::CreateContactMaterialAsset(
				ImportedRoot.GetActorLabel(), ContactMaterial, Material1, Material2);
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

		AAGX_RigidBodyActor* GetActor(const FRigidBodyBarrier& Body)
		{
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
		AActor& ImportedRoot;
		UWorld& World;
		TMap<FGuid, UStaticMesh*> MeshAssets;

		// Map from Guid/Uuid of the AGX Dynamics body provided by the FAGXArchiveReader to the
		// AActor that we created for that body.
		TMap<FGuid, AAGX_RigidBodyActor*> Bodies;
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

	EditorInstantiator Instantiator(*ImportGroup, *World);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE
