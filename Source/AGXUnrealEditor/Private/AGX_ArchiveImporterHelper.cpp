#include "AGX_ArchiveImporterHelper.h"

// AGX Dynamics for Unreal includes.
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
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Tires/AGX_TwoBodyTireActor.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"

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
	// Only instantiate body if it has not already been instantiated. It might have been
	// instantiated already during import of e.g. Tire model.
	if (GetBody(Barrier, false) != nullptr)
	{
		return nullptr;
	}

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
	// Only instantiate body if it has not already been instantiated. It might have been
	// instantiated already during import of e.g. Tire model.
	if (GetBody(Barrier, false) != nullptr)
	{
		return nullptr;
	}

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
	UMaterialInterface* CreateRenderMaterialInstance(
		const FAGX_RenderMaterial& RenderMaterial, const FString& DirectoryName,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials)
	{
		FGuid Guid = RenderMaterial.Guid;

		// Have we seen this render material before?
		if (UMaterialInstanceConstant** It = RestoredMaterials.Find(Guid))
		{
			// Yes, used the cached Material Instance.
			return *It;
		}

		// This is a new material. Save it as an asset and in the cache.
		FString MaterialName = RenderMaterial.Name.IsNone()
								   ? FString::Printf(TEXT("RenderMaterial_%s"), *Guid.ToString())
								   : RenderMaterial.Name.ToString();

		UMaterialInterface* Material = FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
			RenderMaterial, DirectoryName, MaterialName);
		if (Material == nullptr)
		{
			return nullptr;
		}

		if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Material))
		{
			RestoredMaterials.Add(RenderMaterial.Guid, Instance);
		}

		return Material;
	}

	void CreateRenderMaterialInstance(
		UMeshComponent& Component, const FAGX_RenderMaterial& RenderMaterial,
		const FString& DirectoryName, TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials)
	{
		FGuid Guid = RenderMaterial.Guid;

		// Have we seen this render material before?
		if (UMaterialInstanceConstant** It = RestoredMaterials.Find(Guid))
		{
			// Yes, use the cached Material Instance.
			check(*It != nullptr); // Should never put nullptr into the table.
			Component.SetMaterial(0, *It);
			return;
		}

		// It's a new material, save it as an asset and in the cache.
		FString MaterialName =
			RenderMaterial.Name.IsNone() ? Component.GetName() : RenderMaterial.Name.ToString();
		UMaterialInterface* Material = FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
			RenderMaterial, DirectoryName, MaterialName);
		if (Material == nullptr)
		{
			// Fallback to the default import material is done by SaveImportedRenderMaterialAsset,
			// so no need to try and call SetDefaultRenderMaterial here.
			UE_LOG(
				LogAGX, Warning, TEXT("Could not set render material on imported shape '%s'."),
				*Component.GetName());
			return;
		}
		if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Material))
		{
			// We don't get here if the save failed and we fell back to the default import material.
			RestoredMaterials.Add(RenderMaterial.Guid, Instance);
		}
		Component.SetMaterial(0, Material);
	}

	UMaterial* GetDefaultRenderMaterial(bool bIsSensor)
	{
		const TCHAR* AssetPath =
			bIsSensor
				? TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'")
				: TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'");
		UMaterial* Material = FAGX_TextureUtilities::GetMaterialFromAssetPath(AssetPath);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Could not load default%s render material from '%s'."),
				(bIsSensor ? TEXT(" sensor") : TEXT("")), AssetPath);
		}
		return Material;
	}

	void SetDefaultRenderMaterial(UMeshComponent& Component, bool bIsSensor)
	{
		UMaterial* Material = GetDefaultRenderMaterial(bIsSensor);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not set render material on imported shape '%s'. Could not load the "
					 "default render material"),
				*Component.GetName());
			return;
		}
		Component.SetMaterial(0, Material);
	}

	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes, const FString& DirectoryName)
	{
		FGuid Guid = Trimesh.GetMeshDataGuid();

		// If the GUID is invalid, try to create the mesh asset anyway but without adding it to the
		// RestoredMeshes map.
		if (!Guid.IsValid())
		{
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Trimesh, DirectoryName, FallbackName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			return Asset;
		}

		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(Trimesh, DirectoryName, FallbackName);
		RestoredMeshes.Add(Guid, Asset);
		return Asset;
	}

	/**
	 * Convert the Render Data to an Unreal Engine Static Mesh asset stored in the RenderMeshes
	 * folder in the archive's folder in the ImportedAGXArchives folder.
	 *
	 * The created meshes are cached on GUID so asking for the same Render Data mesh again will
	 * return the previously created Static Mesh asset.
	 *
	 * @param RenderData The Render Data Barrier containing the mesh to store.
	 * @param RestoredMeshes Static Mesh cache.
	 * @param DirectoryName The name of the folder where all assets for this import is stored.
	 * @return
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FRenderDataBarrier& RenderData, TMap<FGuid, UStaticMesh*>& RestoredMeshes,
		const FString& DirectoryName)
	{
		FGuid Guid = RenderData.GetGuid();
		if (!Guid.IsValid())
		{
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			return Asset;
		}

		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		RestoredMeshes.Add(Guid, Asset);
		return Asset;
	}

	/**
	 * Apply the Barrier's Render Data. This will disable visibility for VisualMesh and instead
	 * create a Static Mesh Component from the triangles, if any, in the Render Data. If there are
	 * no triangles then no Static Mesh Component will be created and this Shape will be invisible.
	 *
	 * If the Render Data has a Render Material then that will be converted to an Unreal Engine
	 * Render Material and applied to both VisualMesh and the newly created Static Mesh Component,
	 * if any. This makes it possible to hide the Render Data mesh and instead use the collision
	 * data also for rendering.
	 *
	 * @param RenderData The AGX Dynamics representation of the Render Data.
	 * @param VisualMesh The default Component used for visualization. Often the Shape itself.
	 */
	void ApplyRenderingData(
		const FRenderDataBarrier& RenderData, UAGX_ShapeComponent& Component,
		UMeshComponent& VisualMesh, TMap<FGuid, UStaticMesh*>& RestoredMeshes,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials, const FString& DirectoryName)
	{
		VisualMesh.SetVisibility(false);

		// Convert Render Data Mesh, if there is one.
		UStaticMeshComponent* RenderDataComponent = nullptr;
		if (RenderData.HasMesh())
		{
			UStaticMesh* RenderDataMeshAsset =
				GetOrCreateStaticMeshAsset(RenderData, RestoredMeshes, DirectoryName);
			RenderDataComponent = FAGX_EditorUtilities::CreateStaticMeshComponent(
				*Component.GetOwner(), Component, *RenderDataMeshAsset);
			RenderDataComponent->SetVisibility(RenderData.GetShouldRender());
		}

		// Convert Render Data Material, if there is one. May fall back to our default Material, and
		// may also fail completely, leaving this at nullptr.
		UMaterialInterface* RenderDataMaterial = nullptr;
		if (RenderData.HasMaterial() && GIsEditor)
		{
			RenderDataMaterial = CreateRenderMaterialInstance(
				RenderData.GetMaterial(), DirectoryName, RestoredMaterials);
		}
		else
		{
			// Use our default render material if the Render Data didn't have one. Also use the
			// default when not in the Editor since creating new Materials is a Editor only
			// operation.
			//
			// We are only allowed to create new assets, such as a MaterialInstance, when running
			// within the Unreal Editor.
			/// @todo This is not true. It seems we are allowed to create StaticMeshes. What's the
			/// difference between a StaticMesh and a MaterialInstance? Is it because we create
			/// Constant Material Instances?
			RenderDataMaterial = GetDefaultRenderMaterial(Component.bIsSensor);
		}

		// Apply the Material we got, either from the Render Data or the default one, to all the
		// rendering meshes we have.
		if (RenderDataMaterial != nullptr)
		{
			VisualMesh.SetMaterial(0, RenderDataMaterial);
			if (RenderDataComponent != nullptr)
			{
				RenderDataComponent->SetMaterial(0, RenderDataMaterial);
			}
		}
	}

	/**
	 * Do the configuration that is common for all shape types. This includes setting object flags,
	 * Component renaming, and applying Physics and Rendering Materials.
	 *
	 * @param Component The Unreal Engine representation of the imported Shape.
	 * @param Barrier The AGX Dynamics representation of the imported Shape.
	 * @param RestoredShapeMaterials Cache of imported Shape Materials.
	 * @param RestoredRenderMaterials Cache of imported Render Materials.
	 * @param RestoredMeshes Cache of imported Meshes, both for collision detection and rendering.
	 * @param DirectoryName The name of the directory where all the imported assets are stored.
	 * @param VisualMesh The Mesh Component that contains the default rendering of the Shape
	 */
	void FinalizeShape(
		UAGX_ShapeComponent& Component, const FShapeBarrier& Barrier,
		const TMap<FGuid, UAGX_ShapeMaterialAsset*>& RestoredShapeMaterials,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredRenderMaterials,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes, const FString& DirectoryName,
		UMeshComponent& VisualMesh)
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

		if (Barrier.HasRenderData())
		{
			ApplyRenderingData(
				Barrier.GetRenderData(), Component, VisualMesh, RestoredMeshes,
				RestoredRenderMaterials, DirectoryName);
		}
		else
		{
			SetDefaultRenderMaterial(VisualMesh, Component.bIsSensor);
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
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
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
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
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
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
}

UAGX_CapsuleShapeComponent* FAGX_ArchiveImporterHelper::InstantiateCapsule(
	const FCapsuleShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	UAGX_CapsuleShapeComponent* Component =
		FAGX_EditorUtilities::CreateCapsuleShape(Body.GetOwner(), &Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Capsule"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new UAGX_CapsuleShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
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
	if (MeshComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create MeshComponent for imported trimesh '%s' in body '%s'."),
			*MeshName, *Body.GetName());
	}
	if (MeshComponent != nullptr)
	{
		FAGX_ImportUtilities::Rename(*MeshComponent, *MeshName);
	}

	// Both components must be created and attached before they are registered because BeginPlay
	// may be called by RegisterComponent and the TrimeshMeshComponent must know of the
	// StaticMeshComponent before that happens.
	/// @todo In which order should these be? Does it matter?
	if (MeshComponent != nullptr)
	{
		MeshComponent->RegisterComponent();
	}
	Component->RegisterComponent();

	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *MeshComponent);
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
		const FBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		if (IgnoreList.Contains(Barrier.GetGuid()))
		{
			// Don't instantiate the Constraint if it is in the ignore list. This might be the case
			// for e.g. the Hinge constraint owned by a native TwoBodyTire object.
			return nullptr;
		}

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

		FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
			Barrier, *Component, Bodies.first, Bodies.second);
		FAGX_ConstraintUtilities::CopyControllersFrom(*Component, Barrier);
		FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
		return Component;
	}

	template <typename UActor, typename FBarrier>
	UActor* InstantiateConstraint(
		const FBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		if (IgnoreList.Contains(Barrier.GetGuid()))
		{
			// Don't instantiate the Constraint if it is in the ignore list. This might be the case
			// for e.g. the Hinge constraint owned by a native TwoBodyTire object.
			return nullptr;
		}

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

		FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
			Barrier, *Actor->GetConstraintComponent(), Bodies.first, Bodies.second);

		/// \todo Make CopyControllersFrom a virtual member function of UAGX_ConstraintComponent.
		/// Then we won't need the code duplication in the functions calling this one.

		// This is where AttachToActor was. Now it's in the ToActorTree class.

		FAGX_ImportUtilities::Rename(*Actor, Barrier.GetName());
		/// \todo Should we call SetActorLabel here?

		return Actor;
	}

	/// \todo Consider removing the 1Dof and 2Dof instantiatior functions. Does not seem to be
	/// needed, just call the generic InstantiateConstraint immediately.

	template <typename UComponent>
	UComponent* InstantiateConstraint1Dof(
		const FConstraint1DOFBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		return InstantiateConstraint<UComponent>(Barrier, Owner, Helper, IgnoreList);
	}

	template <typename UActor>
	UActor* InstantiateConstraint1Dof(
		const FConstraint1DOFBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		UActor* Actor = InstantiateConstraint<UActor>(Barrier, Helper, IgnoreList);
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
		const FConstraint2DOFBarrier& Barrier, AActor& Owner, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		return InstantiateConstraint<UConstraint>(Barrier, Owner, Helper, IgnoreList);
	}

	template <typename UActor>
	UActor* InstantiateConstraint2Dof(
		const FConstraint2DOFBarrier& Barrier, FAGX_ArchiveImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		UActor* Actor = InstantiateConstraint<UActor>(Barrier, Helper, IgnoreList);
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
	return ::InstantiateConstraint1Dof<AAGX_HingeConstraintActor>(
		Barrier, *this, ConstraintIgnoreList);
}

UAGX_HingeConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_HingeConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

AAGX_PrismaticConstraintActor* FAGX_ArchiveImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier)
{
	return ::InstantiateConstraint1Dof<AAGX_PrismaticConstraintActor>(
		Barrier, *this, ConstraintIgnoreList);
}

UAGX_PrismaticConstraintComponent* FAGX_ArchiveImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_PrismaticConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

AAGX_BallConstraintActor* FAGX_ArchiveImporterHelper::InstantiateBallJoint(
	const FBallJointBarrier& Barrier)
{
	return ::InstantiateConstraint<AAGX_BallConstraintActor>(Barrier, *this, ConstraintIgnoreList);
}

UAGX_BallConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateBallJoint(
	const FBallJointBarrier& Barrier, AActor& Owner)
{
	return InstantiateConstraint<UAGX_BallConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

AAGX_CylindricalConstraintActor* FAGX_ArchiveImporterHelper::InstantiateCylindricalJoint(
	const FCylindricalJointBarrier& Barrier)
{
	return ::InstantiateConstraint2Dof<AAGX_CylindricalConstraintActor>(
		Barrier, *this, ConstraintIgnoreList);
}

UAGX_CylindricalConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateCylindricalJoint(
	const FCylindricalJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint2Dof<UAGX_CylindricalConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

AAGX_DistanceConstraintActor* FAGX_ArchiveImporterHelper::InstantiateDistanceJoint(
	const FDistanceJointBarrier& Barrier)
{
	return ::InstantiateConstraint1Dof<AAGX_DistanceConstraintActor>(
		Barrier, *this, ConstraintIgnoreList);
}

UAGX_DistanceConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateDistanceJoint(
	const FDistanceJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_DistanceConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

AAGX_LockConstraintActor* FAGX_ArchiveImporterHelper::InstantiateLockJoint(
	const FLockJointBarrier& Barrier)
{
	return ::InstantiateConstraint<AAGX_LockConstraintActor>(Barrier, *this, ConstraintIgnoreList);
}

UAGX_LockConstraintComponent* FAGX_ArchiveImporterHelper::InstantiateLockJoint(
	const FLockJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint<UAGX_LockConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_TwoBodyTireComponent* FAGX_ArchiveImporterHelper::InstantiateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, AActor& Owner, bool IsBlueprintOwner)
{
	UAGX_TwoBodyTireComponent* Component = NewObject<UAGX_TwoBodyTireComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new AGX_TwoBodyTireComponent"));
		return nullptr;
	}

	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef) {
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), ArchiveFilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.BodyName = Body->GetFName();
		if (!IsBlueprintOwner)
		{
			BodyRef.OwningActor = Body->GetOwner();
		}
	};

	SetRigidBody(GetBody(Barrier.GetTireRigidBody()), Component->TireRigidBody);
	SetRigidBody(GetBody(Barrier.GetHubRigidBody()), Component->HubRigidBody);
	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
	Component->CopyFrom(Barrier);
	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();

	// The internal constraint owned by the TwoBodyTire should not be imported, but is created after
	// BeginPlay by the native TwoBodyTire.
	ConstraintIgnoreList.Add(Barrier.GetHingeGuid());

	return Component;
}

AAGX_TwoBodyTireActor* FAGX_ArchiveImporterHelper::InstantiateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, UWorld& World)
{
	AAGX_TwoBodyTireActor* NewActor =
		World.SpawnActor<AAGX_TwoBodyTireActor>(AAGX_TwoBodyTireActor::StaticClass());
	if (NewActor == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), ArchiveFilePath,
			TEXT("Could not create new AGX_TwoBodyTireActor"));
		return nullptr;
	}

	FAGX_ImportUtilities::Rename(*NewActor, Barrier.GetName());
	NewActor->SetActorLabel(NewActor->GetName());
	NewActor->TwoBodyTireComponent->CopyFrom(Barrier);

	// Setup TireRigidBody and HubRigidBody.
	auto SetupBody = [&](const FRigidBodyBarrier& BodyBarrier, UAGX_RigidBodyComponent* Body) {
		if (BodyBarrier.HasNative() == false)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), ArchiveFilePath,
				TEXT("The referenced Rigid Body did not have a native Rigid Body allocated. The "
					 "TwoBodyTire might not work as expected."));
			return;
		}

		Body->CopyFrom(BodyBarrier);
		RestoredBodies.Add(BodyBarrier.GetGuid(), Body);
	};

	SetupBody(Barrier.GetTireRigidBody(), NewActor->TireRigidBodyComponent);
	SetupBody(Barrier.GetHubRigidBody(), NewActor->HubRigidBodyComponent);

	// The internal constraint owned by the TwoBodyTire should not be imported, but is created after
	// BeginPlay by the native TwoBodyTire.
	ConstraintIgnoreList.Add(Barrier.GetHingeGuid());

	return NewActor;
}

UAGX_CollisionGroupDisablerComponent* FAGX_ArchiveImporterHelper::InstantiateCollisionGroupDisabler(
	AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs)
{
	UAGX_CollisionGroupDisablerComponent* Component =
		NewObject<UAGX_CollisionGroupDisablerComponent>(&Owner, TEXT("AGX_CollisionGroupDisabler"));

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
	{
		Component->DisableCollisionGroupPair(
			FName(*DisabledPair.first), FName(*DisabledPair.second));
	}

	return Component;
}

AAGX_CollisionGroupDisablerActor* FAGX_ArchiveImporterHelper::InstantiateCollisionGroupDisabler(
	UWorld& World, const TArray<std::pair<FString, FString>>& DisabledPairs)
{
	AAGX_CollisionGroupDisablerActor* NewActor = World.SpawnActor<AAGX_CollisionGroupDisablerActor>(
		AAGX_CollisionGroupDisablerActor::StaticClass());
	if (NewActor == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics CollisionGroupDisabler"), "", ArchiveFilePath,
			TEXT("Could not create new AGX_CollisionGroupDisablerActor"));
		return nullptr;
	}

	for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
	{
		NewActor->CollisionGroupDisablerComponent->DisableCollisionGroupPair(
			FName(*DisabledPair.first), FName(*DisabledPair.second));
	}

	return NewActor;
}

UAGX_RigidBodyComponent* FAGX_ArchiveImporterHelper::GetBody(
	const FRigidBodyBarrier& Barrier, bool LogErrorIfNotFound)
{
	/// \todo Callers cannot differentiate between a nullptr return because the Barrier really
	/// represents a nullptr body, and a nullptr return because the AGXUnreal representation of an
	/// existing Barrier body couldn't be found. This may cause e.g. constraints that should be
	/// between two bodies to be between a body and the world instead. An error message will be
	/// printed, however, so the user will know what happened, if they read the log.

	if (!Barrier.HasNative())
	{
		// Not an error for constraints. Means that the other body is constrained to the world.
		return nullptr;
	}

	UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Barrier.GetGuid());
	if (Component == nullptr && LogErrorIfNotFound)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("While importing from '%s': A component references a body '%s', but that body "
				 "hasn't been restored."),
			*ArchiveFilePath, *Barrier.GetName());
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
	return {
		GetShapeMaterial(ContactMaterial.GetMaterial1()),
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
			/// somewhere, where?
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
