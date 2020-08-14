#include "AGX_ArchiveImporterHelper.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "RigidBodyBarrier.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_BallConstraintActor.h"
#include "Constraints/AGX_CylindricalConstraintActor.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintActor.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintActor.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintActor.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintActor.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ConstraintUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	void WriteImportErrorMessage(
		const TCHAR* ObjectType, const FString& Name, const FString& ArchiveFilePath,
		const TCHAR* Message)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not import %s '%s' from AGX Dynamics archive '%s': %s."),
			ObjectType, *Name, *ArchiveFilePath, Message);
	}
};

UAGX_RigidBodyComponent* FAGX_ArchiveImporterHelper::InstantiateBody(
	const FRigidBodyBarrier& Barrier, AActor& Actor)
{
	UAGX_RigidBodyComponent* Component = NewObject<UAGX_RigidBodyComponent>(&Actor);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics RigidBody"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new AGX_RigidBodyComponent"));
		return nullptr;
	}
	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
	Component->CopyFrom(Barrier);
	Component->SetFlags(RF_Transactional);
	Actor.AddInstanceComponent(Component);

	/// @todo What does this do, really? Are we required to call it? A side effect of this is that
	/// BeginPlay is called, which in turn calls AllocateNative. Which means that an AGX Dynamics
	/// RigidBody is created. I'm not sure if this is consistent with AGX_RigidBodyComponents
	/// created with using the Editor's Add Component button for an Actor in the Level Viewport.
	/// <investigating>
	/// ActorComponent.cpp, RegisterComponentWithWorld, has the following code snippet, somewhat
	/// simplified:
	///
	/// if (!InWorld->IsGameWorld())
	/// {}
	/// else if (MyOwner == nullptr)
	/// {}
	/// else
	/// {
	///    if (MyOwner->HasActorBegunPlay() && !bHasBegunPlay)
	///    {
	///        BeginPlay();
	///     }
	/// }
	///
	/// So, BeginPlay is only called if we don't have a Game world (have Editor world, for example)
	/// and the owning Actor have had its BeginPlay called already.
	///
	/// This makes the Editor situation different from the Automation Test situation since the
	/// Editor has an Editor world and Automation Tests run with a Game world. So creating an
	/// AGX_RigidBodyComponent in the editor does not trigger BeginPlay, but creating an
	/// AGX_RigidBody while importing an AGX Dynamics archive during an Automation Test does trigger
	/// BeginPlay here. Not sure if this is a problem or not, but something to be aware of.
	Component->RegisterComponent();

	Component->PostEditChange();
	RestoredBodies.Add(Barrier.GetGuid(), Component);
	return Component;
}

AAGX_RigidBodyActor* FAGX_ArchiveImporterHelper::InstantiateBody(
	const FRigidBodyBarrier& Barrier, UWorld& World)
{
	FTransform Transform(Barrier.GetRotation(), Barrier.GetPosition());
	AAGX_RigidBodyActor* NewActor =
		World.SpawnActor<AAGX_RigidBodyActor>(AAGX_RigidBodyActor::StaticClass(), Transform);
	if (NewActor == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics RigidBody"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new AGX_RigidBodyActor"));
		return nullptr;
	}
	FAGX_ImportUtilities::Rename(*NewActor, Barrier.GetName());
	NewActor->SetActorLabel(NewActor->GetName());
	NewActor->RigidBodyComponent->CopyFrom(Barrier);
	RestoredBodies.Add(Barrier.GetGuid(), NewActor->RigidBodyComponent);
	/// \todo Do we need to do any additional configuration here?
	return NewActor;
}

namespace
{
	void CreateRenderMaterialInstance(
		UAGX_ShapeComponent& Component, const FAGX_RenderData& RenderData,
		const FString& DirectoryName)
	{
		FString MaterialName = Component.GetName();
		UMaterialInstanceConstant* Material = FAGX_ImportUtilities::SaveImportedRenderDataAsset(
			RenderData, DirectoryName, MaterialName);
		Component.SetMaterial(0, Material);
	}

	void SetDefaultRenderMaterial(UAGX_ShapeComponent& Component)
	{
		const TCHAR* AssetPath =
			TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'");
		UObject* LoadResult = StaticLoadObject(UMaterial::StaticClass(), nullptr, AssetPath);
		if (LoadResult == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not set render material on imported shape '%s'. The material '%s' "
					 "could not be loaded."),
				*Component.GetName(), AssetPath);
			return;
		}
		UMaterial* Material = Cast<UMaterial>(LoadResult);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not set render material on imported shape '%s'. The asset '%s' is not "
					 "a Materil."),
				*Component.GetName(), AssetPath);
		}
		Component.SetMaterial(0, Material);
	}

	void FinalizeShape(
		UAGX_ShapeComponent& Component, const FShapeBarrier& Barrier,
		const TMap<FGuid, UAGX_ShapeMaterialAsset*>& RestoredShapeMaterials,
		const FString& DirectoryName)
	{
		Component.UpdateVisualMesh();
		Component.SetFlags(RF_Transactional);
		FAGX_ImportUtilities::Rename(Component, Barrier.GetName());

		FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
		if (NativeMaterial.HasNative())
		{
			FGuid Guid = NativeMaterial.GetGuid();
			UAGX_ShapeMaterialAsset* Material = RestoredShapeMaterials.FindRef(Guid);
			Component.PhysicalMaterial = Material;
		}

		if (Barrier.HasRenderData() && GIsEditor)
		{
			// We are only allowed to create new assets, such as a MaterialInstance, when running
			// within the Unreal Editor.
			CreateRenderMaterialInstance(Component, Barrier.GetRenderData(), DirectoryName);
		}
		else
		{
			SetDefaultRenderMaterial(Component);
		}
	}
}

UAGX_SphereShapeComponent* FAGX_ArchiveImporterHelper::InstantiateSphere(
	const FSphereShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	UAGX_SphereShapeComponent* Component =
		FAGX_EditorUtilities::CreateSphereShape(Body.GetOwner(), &Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Sphere"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new UAGX_SphereShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials, DirectoryName);
	return Component;
}

UAGX_BoxShapeComponent* FAGX_ArchiveImporterHelper::InstantiateBox(
	const FBoxShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	UAGX_BoxShapeComponent* Component =
		FAGX_EditorUtilities::CreateBoxShape(Body.GetOwner(), &Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Box"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new UAGX_BoxShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials, DirectoryName);
	return Component;
}

UAGX_CylinderShapeComponent* FAGX_ArchiveImporterHelper::InstantiateCylinder(
	const FCylinderShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	UAGX_CylinderShapeComponent* Component =
		FAGX_EditorUtilities::CreateCylinderShape(Body.GetOwner(), &Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Cylinder"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new UAGX_CylinderShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials, DirectoryName);
	return Component;
}

namespace
{
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Barrier, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*> RestoredMeshes, const FString& DirectoryName)
	{
		FGuid Guid = Barrier.GetMeshDataGuid();

		// If the GUID is invalid, try to create the mesh asset anyway but without adding it to the
		// RestoredMeshes map.
		if (!Guid.IsValid())
		{
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Barrier, DirectoryName, FallbackName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			return Asset;
		}

		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(Barrier, DirectoryName, FallbackName);
		RestoredMeshes.Add(Guid, Asset);
		return Asset;
	}
}

UAGX_TrimeshShapeComponent* FAGX_ArchiveImporterHelper::InstantiateTrimesh(
	const FTrimeshShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	AActor* Owner = Body.GetOwner();
	UAGX_TrimeshShapeComponent* Component =
		FAGX_EditorUtilities::CreateTrimeshShape(Owner, &Body, false);
	Component->MeshSourceLocation = EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
	UStaticMesh* MeshAsset =
		GetOrCreateStaticMeshAsset(Barrier, Body.GetName(), RestoredMeshes, DirectoryName);
	if (MeshAsset == nullptr)
	{
		// No point in continuing further. Logging handled in GetOrCreateStaticMeshAsset.
		/// \todo Consider moving logging in here, using WriteImportErrorMessage.
		return nullptr;
	}

	UStaticMeshComponent* MeshComponent =
		FAGX_EditorUtilities::CreateStaticMeshComponent(Owner, Component, MeshAsset, false);
	FString SourceName = Barrier.GetSourceName();
	FString MeshName = !SourceName.IsEmpty() ? SourceName : (Barrier.GetName() + TEXT("Mesh"));
	FAGX_ImportUtilities::Rename(*MeshComponent, *MeshName);

	/// @todo In which order should these be?
	MeshComponent->RegisterComponent();
	Component->RegisterComponent();

	Component->CopyFrom(Barrier);
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials, DirectoryName);
	return Component;
}

UAGX_ShapeMaterialAsset* FAGX_ArchiveImporterHelper::InstantiateShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	/// \todo Do we need any special handling of the default material?
	UAGX_ShapeMaterialAsset* Asset =
		FAGX_ImportUtilities::SaveImportedShapeMaterialAsset(Barrier, DirectoryName);
	RestoredShapeMaterials.Add(Barrier.GetGuid(), Asset);
	return Asset;
}

UAGX_ContactMaterialAsset* FAGX_ArchiveImporterHelper::InstantiateContactMaterial(
	const FContactMaterialBarrier& Barrier)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	UAGX_ContactMaterialAsset* Asset = FAGX_ImportUtilities::SaveImportedContactMaterialAsset(
		Barrier, Materials.first, Materials.second, DirectoryName);
	return Asset;
}

namespace
{
	template <typename UComponent, typename FBarrier>
	UComponent* InstantiateConstraint(
		const FBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper)
	{
		FAGX_ArchiveImporterHelper::FBodyPair Bodies = Helper.GetBodies(Barrier);
		if (Bodies.first == nullptr)
		{
			// Not having a second body is fine, means that the first body is constrained to the
			// world. Not having a first body is bad.
			UE_LOG(
				LogAGX, Warning,
				TEXT("Constraint '%s' imported from '%s' does not have a first body. Ignoring."),
				*Barrier.GetName(), *Helper.ArchiveFilePath);
			return nullptr;
		}

		UComponent* Component = FAGX_EditorUtilities::CreateConstraintComponent<UComponent>(
			&Owner, Bodies.first, Bodies.second);
		if (Component == nullptr)
		{
			return nullptr;
		}

		FAGX_ConstraintUtilities::StoreFrames(Barrier, *Component);
		FAGX_ConstraintUtilities::CopyControllersFrom(*Component, Barrier);
		FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
		return Component;
	}

	template <typename UActor, typename FBarrier>
	UActor* InstantiateConstraint(const FBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper)
	{
		FAGX_ArchiveImporterHelper::FBodyPair Bodies = Helper.GetBodies(Barrier);
		if (Bodies.first == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("Hinge"), Barrier.GetName(), Helper.ArchiveFilePath,
				TEXT("The constraint contains a reference to an unknown body"));
			return nullptr;
		}

		UActor* Actor = FAGX_EditorUtilities::CreateConstraintActor<UActor>(
			Bodies.first, Bodies.second, false, false, false);
		/// \todo Check for nullptr;

		FAGX_ConstraintUtilities::StoreFrames(Barrier, *Actor->GetConstraintComponent());

		/// \todo Make CopyControllersFrom a virtual member function of UAGX_ConstraintComponent.
		/// Then we won't need the code duplication in the functions calling this one.

		/// \todo Compute the inverse of the first body's attachment frame. That will place the
		/// Actor at the right spot relative to that body.
		Actor->SetActorTransform(Bodies.first->GetComponentTransform());

		// This is where AttachToActor was. Now it's in the ToActorTree class.

		FAGX_ImportUtilities::Rename(*Actor, Barrier.GetName());
		/// \todo Should we call SetActorLabel here?

		return Actor;
	}

	/// \todo Consider removing the 1Dof and 2Dof instantiatior functions. Does not seem to be
	/// needed, just call the generic InstantiateConstraint immediately.

	template <typename UComponent>
	UComponent* InstantiateConstraint1Dof(
		const FConstraint1DOFBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper)
	{
		return InstantiateConstraint<UComponent>(Barrier, Owner, Helper);
	}

	template <typename UActor>
	UActor* InstantiateConstraint1Dof(
		const FConstraint1DOFBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper)
	{
		UActor* Actor = InstantiateConstraint<UActor>(Barrier, Helper);
		if (Actor == nullptr)
		{
			// No need to log here, done by InstantiateConstraint.
			return nullptr;
		}
		FAGX_ConstraintUtilities::CopyControllersFrom(*Actor->Get1DofComponent(), Barrier);
		return Actor;
	}

	template <typename UConstraint>
	UConstraint* InstantiateConstraint2Dof(
		const FConstraint2DOFBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper)
	{
		return InstantiateConstraint<UConstraint>(Barrier, Owner, Helper);
	}

	template <typename UActor>
	UActor* InstantiateConstraint2Dof(
		const FConstraint2DOFBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper)
	{
		UActor* Actor = InstantiateConstraint<UActor>(Barrier, Helper);
		if (Actor == nullptr)
		{
			// No need to log here, done by InstantiateConstraint.
			return nullptr;
		}
		FAGX_ConstraintUtilities::CopyControllersFrom(*Actor->Get2DofComponent(), Barrier);
		return Actor;
	}
}

AAGX_HingeConstraintActor* FAGX_ArchiveImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier)
{
	return ::InstantiateConstraint1Dof<AAGX_HingeConstraintActor>(Barrier, *this);
}

UAGX_HingeConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_HingeConstraintComponent>(Barrier, Owner, *this);
}

AAGX_PrismaticConstraintActor* FAGX_ArchiveImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier)
{
	return ::InstantiateConstraint1Dof<AAGX_PrismaticConstraintActor>(Barrier, *this);
}

UAGX_PrismaticConstraintComponent* FAGX_ArchiveImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_PrismaticConstraintComponent>(Barrier, Owner, *this);
}

AAGX_BallConstraintActor* FAGX_ArchiveImporterHelper::InstantiateBallJoint(
	const FBallJointBarrier& Barrier)
{
	return ::InstantiateConstraint<AAGX_BallConstraintActor>(Barrier, *this);
}

UAGX_BallConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateBallJoint(
	const FBallJointBarrier& Barrier, AActor& Owner)
{
	return InstantiateConstraint<UAGX_BallConstraintComponent>(Barrier, Owner, *this);
}

AAGX_CylindricalConstraintActor* FAGX_ArchiveImporterHelper::InstantiateCylindricalJoint(
	const FCylindricalJointBarrier& Barrier)
{
	return ::InstantiateConstraint2Dof<AAGX_CylindricalConstraintActor>(Barrier, *this);
}

UAGX_CylindricalConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateCylindricalJoint(
	const FCylindricalJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint2Dof<UAGX_CylindricalConstraintComponent>(Barrier, Owner, *this);
}

AAGX_DistanceConstraintActor* FAGX_ArchiveImporterHelper::InstantiateDistanceJoint(
	const FDistanceJointBarrier& Barrier)
{
	return ::InstantiateConstraint1Dof<AAGX_DistanceConstraintActor>(Barrier, *this);
}

UAGX_DistanceConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateDistanceJoint(
	const FDistanceJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_DistanceConstraintComponent>(Barrier, Owner, *this);
}

AAGX_LockConstraintActor* FAGX_ArchiveImporterHelper::InstantiateLockJoint(
	const FLockJointBarrier& Barrier)
{
	return ::InstantiateConstraint<AAGX_LockConstraintActor>(Barrier, *this);
}

UAGX_LockConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateLockJoint(
	const FLockJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint<UAGX_LockConstraintComponent>(Barrier, Owner, *this);
}

UAGX_RigidBodyComponent* FAGX_ArchiveImporterHelper::GetBody(const FRigidBodyBarrier& Barrier)
{
	/// \todo Calles cannot differentiate between a nullptr return because the Barrier really
	/// represents a nullptr body, and a nullptr return because the AGXUnreal representation of an
	/// existing Barrier body couldn't be found. This may cause constraints that should be between
	/// to bodies to be between a body and the world instead. A warning will be printed, however, so
	/// the user will know what happened, of they read warnings.

	if (!Barrier.HasNative())
	{
		// Not an error for constraints. Means that the other body is constrained to the world.
		return nullptr;
	}

	UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Barrier.GetGuid());
	if (Component == nullptr)
	{
		/// \todo Consider moving this error message to the constraint importer code.
		UE_LOG(
			LogAGX, Error,
			TEXT("While importing from '%s': Found a constraint to body '%s', but that body hasn't "
				 "been restored."),
			*ArchiveFilePath, *Barrier.GetName());
		return nullptr;
	}

	return Component;
}

FAGX_ArchiveImporterHelper::FBodyPair FAGX_ArchiveImporterHelper::GetBodies(
	const FConstraintBarrier& Barrier)
{
	return {GetBody(Barrier.GetFirstBody()), GetBody(Barrier.GetSecondBody())};
}

UAGX_ShapeMaterialAsset* FAGX_ArchiveImporterHelper::GetShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	return RestoredShapeMaterials.FindRef(Barrier.GetGuid());
}

FAGX_ArchiveImporterHelper::FShapeMaterialPair FAGX_ArchiveImporterHelper::GetShapeMaterials(
	const FContactMaterialBarrier& ContactMaterial)
{
	return {GetShapeMaterial(ContactMaterial.GetMaterial1()),
			GetShapeMaterial(ContactMaterial.GetMaterial2())};
}

namespace
{
	FString MakeArchiveName(FString ArchiveFilename)
	{
		ArchiveFilename.RemoveFromEnd(TEXT(".agx"));
		return FAGX_EditorUtilities::SanitizeName(ArchiveFilename, TEXT("ImportedAgxArchive"));
	}

	FString MakeDirectoryName(const FString ArchiveName)
	{
		FString BasePath = FAGX_ImportUtilities::CreateArchivePackagePath(ArchiveName);

		auto PackageExists = [&](const FString& DirPath) {
			/// @todo Is this check necessary? Can it be something less crashy? It was copied from
			/// somewehre, where?
			check(!FEditorFileUtils::IsMapPackageAsset(DirPath));

			FString DiskPath = FPackageName::LongPackageNameToFilename(DirPath);
			return FPackageName::DoesPackageExist(DirPath) ||
				   FindObject<UPackage>(nullptr, *DirPath) != nullptr ||
				   FPaths::DirectoryExists(DiskPath) || FPaths::FileExists(DiskPath);
		};

		int32 TryCount = 0;
		FString DirectoryPath = BasePath;
		FString DirectoryName = ArchiveName;
		while (PackageExists(DirectoryPath))
		{
			++TryCount;
			DirectoryPath = BasePath + TEXT("_") + FString::FromInt(TryCount);
			DirectoryName = ArchiveName + TEXT("_") + FString::FromInt(TryCount);
		}
		UE_LOG(
			LogAGX, Display, TEXT("Importing AGX Dynamics archive '%s' to '%s'."), *ArchiveName,
			*DirectoryPath);
		return DirectoryName;
	}
}

FAGX_ArchiveImporterHelper::FAGX_ArchiveImporterHelper(const FString& InArchiveFilePath)
	: ArchiveFilePath(InArchiveFilePath)
	, ArchiveFileName(FPaths::GetBaseFilename(InArchiveFilePath))
	, ArchiveName(MakeArchiveName(ArchiveFileName))
	, DirectoryName(MakeDirectoryName(ArchiveName))
{
}
