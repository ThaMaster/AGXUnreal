// Copyright 2022, Algoryx Simulation AB.

#include "AGX_SimObjectsImporterHelper.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_AmorEnums.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"
#include "RigidBodyBarrier.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
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
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"
#include "Wire/AGX_WireComponent.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackPropertiesBarrier.h"
#include "Vehicle/TrackWheelBarrier.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MeshDescription.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	void WriteImportErrorMessage(
		const TCHAR* ObjectType, const FString& Name, const FString& FilePath, const TCHAR* Message)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not import '%s' '%s' from file '%s': %s."), ObjectType,
			*Name, *FilePath, Message);
	}

	UAGX_TrackProperties* GetOrCreateTrackPropertiesAsset(
		const FTrackPropertiesBarrier& Barrier, const FString& Name,
		TMap<FGuid, UAGX_TrackProperties*>& RestoredTrackProperties, const FString& DirectoryName)
	{
		const FGuid Guid = Barrier.GetGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the asset anyway but without adding it to
			// the RestoredTrackProperties cache.
			return FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(
				Barrier, DirectoryName, Name);
		}

		if (UAGX_TrackProperties* Asset = RestoredTrackProperties.FindRef(Guid))
		{
			// We have seen this asset before, use the one in the cache.
			return Asset;
		}

		// This is a new Track Properties. Create the asset and add to the cache.
		UAGX_TrackProperties* Asset =
			FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(Barrier, DirectoryName, Name);
		if (Asset != nullptr)
		{
			RestoredTrackProperties.Add(Guid, Asset);
		}
		return Asset;
	}

	FString GetName(UAGX_ShapeMaterial* Material)
	{
		if (Material == nullptr)
		{
			return TEXT("Default");
		}
		return Material->GetName();
	}

	/**
	 * Convert an AGX Dynamics Render Material to an Unreal Engine Render Material and store it
	 * as an asset in the given directory. Will cache and reuse Render Materials if the same one
	 * is passed multiple times. Will fall back to the base import material if asset creation
	 * fails. Will return nullptr if the base import material can't be loaded.
	 *
	 * If a new Render Material is created then it is created as a Material Instance Constant
	 * from the base import material.
	 *
	 * @param RenderMaterial The AGX Dynamics Material to convert to an Unreal Engine Material.
	 * @param DirectoryName The name of the directory where this imported model's assets are stored.
	 * @param RestoredMaterials Cache of restored Render Materials.
	 * @return The Unreal Engine material for the AGX Dynamics material, or the base material, or
	 * nullptr.
	 */
	UMaterialInterface* GetOrCreateRenderMaterialInstance(
		const FAGX_RenderMaterial& RenderMaterial, const FString& DirectoryName,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials)
	{
		const FGuid Guid = RenderMaterial.Guid;

		// Have we seen this render material before?
		if (UMaterialInstanceConstant** It = RestoredMaterials.Find(Guid))
		{
			// Yes, used the cached Material Instance.
			return *It;
		}

		// This is a new material. Save it as an asset and in the cache.
		const FString MaterialName =
			RenderMaterial.Name.IsNone()
				? FString::Printf(TEXT("RenderMaterial_%s"), *Guid.ToString())
				: RenderMaterial.Name.ToString();
		UMaterialInterface* Material = FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
			RenderMaterial, DirectoryName, MaterialName);
		if (Material == nullptr)
		{
			// Both asset creation and default material load failed. That's bad.
			return nullptr;
		}

		// Check if we got a new Material Instance, or if we fell back to the default material.
		if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Material))
		{
			// This is a new Material Instance, store it in the cache.
			RestoredMaterials.Add(Guid, Instance);
		}

		return Material;
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

	/**
	 * Convert the given Trimesh to an Unreal Engine Static Mesh asset.
	 *
	 * The created meshes are cached on the Trimesh's Mesh Data GUID so asking for the same mesh
	 * again will return the previously created Static Mesh asset.
	 *
	 * @param Trimesh The Trimesh containing the mesh to store.
	 * @param FallbackName A name to give the asset in case the Trimesh doesn't have a valid name.
	 * @param RestoredMeshes Static Mesh cache.
	 * @param DirectoryName The name of the folder where all assets for this imported model is
	 * stored.
	 * @return
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes, const FString& DirectoryName)
	{
		const FGuid Guid = Trimesh.GetMeshDataGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the RestoredMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Trimesh, DirectoryName, FallbackName);
		}

		if (RestoredMeshes.Contains(Guid))
		{
			// We have seen this mesh before, use the one in the cache.
			return RestoredMeshes[Guid];
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(Trimesh, DirectoryName, FallbackName);
		if (Asset != nullptr)
		{
			RestoredMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	/**
	 * Convert the given Render Data to an Unreal Engine Static Mesh asset.
	 *
	 * The created meshes are cached on GUID so asking for the same Render Data mesh again will
	 * return the previously created Static Mesh asset.
	 *
	 * @param RenderData The Render Data Barrier containing the mesh to store.
	 * @param RestoredMeshes Static Mesh cache.
	 * @param DirectoryName The name of the folder where all assets for the imported model is
	 * stored.
	 * @return The Static Mesh asset for the given Render Data.
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FRenderDataBarrier& RenderData, TMap<FGuid, UStaticMesh*>& RestoredMeshes,
		const FString& DirectoryName)
	{
		const FGuid Guid = RenderData.GetGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the RestoredMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		}

		if (RestoredMeshes.Contains(Guid))
		{
			// We have seen this mesh before, use the one in the cache.
			return RestoredMeshes[Guid];
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		if (Asset != nullptr)
		{
			RestoredMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	/*
	 * Creates a RenderMaterial from a RenderDataBarrier if it has a material. Otherwise returns
	 * the default RenderMaterial.
	 */
	UMaterialInterface* CreateRenderMaterialFromRenderDataOrDefault(
		const FRenderDataBarrier& RenderDataBarrier, bool IsSensor, const FString& DirectoryName,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredRenderMaterials)
	{
		// Create the RenderMaterial (if any).
		// Convert Render Data Material, if there is one. May fall back to the base import Material,
		// and may also fail completely, leaving RenderDataMaterial being nullptr.
		UMaterialInterface* RenderDataMaterial = nullptr;
		if (RenderDataBarrier.HasMaterial() && GIsEditor)
		{
			RenderDataMaterial = GetOrCreateRenderMaterialInstance(
				RenderDataBarrier.GetMaterial(), DirectoryName, RestoredRenderMaterials);
		}
		else
		{
			RenderDataMaterial = GetDefaultRenderMaterial(IsSensor);
		}

		return RenderDataMaterial;
	}

	FString CreateMergeSplitThresholdsAssetName(EAGX_AmorOwningType OwningType, const FGuid& Guid)
	{
		switch (OwningType)
		{
			case EAGX_AmorOwningType::BodyOrShape:
				return "AGX_SMST_" + Guid.ToString();
			case EAGX_AmorOwningType::Constraint:
				return "AGX_CMST_" + Guid.ToString();
			case EAGX_AmorOwningType::Wire:
				return "AGX_WMST_" + Guid.ToString();
		}

		UE_LOG(LogAGX, Warning, TEXT("Unknown OwningType in CreateMergeSplitThresholdsAssetName."));
		return "AGX_MST_" + Guid.ToString();
	}

	void UpdateAndSaveMergeSplitThresholdsAsset(
		const FMergeSplitThresholdsBarrier& Barrier, UAGX_MergeSplitThresholdsBase& Asset,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& RestoredThresholds,
		EAGX_AmorOwningType OwningType)
	{
		const FGuid Guid = Barrier.GetGuid();
		const FString AssetName = CreateMergeSplitThresholdsAssetName(OwningType, Guid);

		FAGX_EditorUtilities::RenameAsset(Asset, AssetName, "AGX_MST");
		Asset.CopyFrom(Barrier);
		FAGX_ObjectUtilities::SaveAsset(Asset);
		if (Guid.IsValid())
		{
			RestoredThresholds.Add(Guid, &Asset);
		}
	}

	template <typename TBarrier, typename TThresholdsBarrier>
	UAGX_MergeSplitThresholdsBase* GetOrCreateMergeSplitThresholdsAsset(
		const TBarrier& Barrier, EAGX_AmorOwningType OwningType,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& RestoredThresholds,
		const FString& DirectoryName)
	{
		const TThresholdsBarrier ThresholdsBarrier = TThresholdsBarrier::CreateFrom(Barrier);
		if (!ThresholdsBarrier.HasNative())
		{
			// The native object did not have any MergeSplitThreshold associated with it.
			return nullptr;
		}

		const FGuid Guid = ThresholdsBarrier.GetGuid();
		const FString AssetName = CreateMergeSplitThresholdsAssetName(OwningType, Guid);
		auto CreateAsset = [&]() -> UAGX_MergeSplitThresholdsBase*
		{
			const FString MSTDir =
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();
			switch (OwningType)
			{
				case EAGX_AmorOwningType::BodyOrShape:
					return FAGX_ImportUtilities::CreateAsset<UAGX_ShapeContactMergeSplitThresholds>(
						DirectoryName, AssetName, MSTDir);
				case EAGX_AmorOwningType::Constraint:
					return FAGX_ImportUtilities::CreateAsset<UAGX_ConstraintMergeSplitThresholds>(
						DirectoryName, AssetName, MSTDir);
				case EAGX_AmorOwningType::Wire:
					return FAGX_ImportUtilities::CreateAsset<UAGX_WireMergeSplitThresholds>(
						DirectoryName, AssetName, MSTDir);
				default:
					return nullptr;
			}
		};

		if (UAGX_MergeSplitThresholdsBase* Asset = RestoredThresholds.FindRef(Guid))
		{
			// We have seen this before, use the one in the cache.
			return Asset;
		}

		// This is a new merge split thresholds. Create the asset and add to the cache.
		UAGX_MergeSplitThresholdsBase* Asset = CreateAsset();
		AGX_CHECK(Asset != nullptr);
		UpdateAndSaveMergeSplitThresholdsAsset(
			ThresholdsBarrier, *Asset, RestoredThresholds, OwningType);

		return Asset;
	}

	UAGX_ContactMaterialRegistrarComponent* GetOrCreateContactMaterialRegistrar(AActor& Owner)
	{
		UAGX_ContactMaterialRegistrarComponent* Component =
			Owner.FindComponentByClass<UAGX_ContactMaterialRegistrarComponent>();

		if (Component != nullptr)
		{
			return Component;
		}

		// No UAGX_ContactMaterialRegistrarComponent exists in Owner. Create and add one.
		Component = NewObject<UAGX_ContactMaterialRegistrarComponent>(
			&Owner, TEXT("AGX_ContactMaterialRegistrar"));

		Component->SetFlags(RF_Transactional);
		Owner.AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	bool IsMeshEquivalent(const FRenderDataBarrier& RenderDataBarrier, UStaticMesh* StaticMesh)
	{
		if (StaticMesh == nullptr)
		{
			return false;
		}

		// @todo: can we match the meshes in a fast way? The vertex count does not generally match
		// apparently, probably because UE does some optimizations when building the original
		// StaticMesh.
#if 0
		FMeshDescription* MeshDescr = StaticMesh->GetMeshDescription(0);
		const auto& Vertices = MeshDescr->Vertices();
		// etc...
#endif
		return false;
	}

	bool IsMeshEquivalent(const FTrimeshShapeBarrier& TrimeshBarrier, UStaticMesh* StaticMesh)
	{
		// @todo : see comment in IsMeshEquivalent(const FRenderDataBarrier& RenderDataBarrier,
		// UStaticMesh* StaticMesh) above.
		return false;
	}
}

void FAGX_SimObjectsImporterHelper::UpdateRigidBodyComponent(
	const FRigidBodyBarrier& Barrier, UAGX_RigidBodyComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());
	Component.CopyFrom(Barrier);

	const FMergeSplitThresholdsBarrier ThresholdsBarrier =
		FShapeContactMergeSplitThresholdsBarrier::CreateFrom(Barrier);
	const FGuid MSTGuid = Barrier.GetGuid();
	UAGX_MergeSplitThresholdsBase* MSThresholds = nullptr;
	if (MSTsOnDisk.Contains(MSTGuid) && ThresholdsBarrier.HasNative())
	{
		MSThresholds = MSTsOnDisk[MSTGuid];
		::UpdateAndSaveMergeSplitThresholdsAsset(
			ThresholdsBarrier, *MSThresholds, RestoredThresholds, EAGX_AmorOwningType::BodyOrShape);
	}
	else
	{
		MSThresholds = ::GetOrCreateMergeSplitThresholdsAsset<
			FRigidBodyBarrier, FShapeContactMergeSplitThresholdsBarrier>(
			Barrier, EAGX_AmorOwningType::BodyOrShape, RestoredThresholds, DirectoryName);
	}

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			if (Instance->MergeSplitProperties.Thresholds ==
				Component.MergeSplitProperties.Thresholds)
			{
				Instance->MergeSplitProperties.Thresholds =
					Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
			}
		}
	}

	Component.MergeSplitProperties.Thresholds =
		Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);

	AGX_CHECK(!RestoredBodies.Contains(Barrier.GetGuid()));
	RestoredBodies.Add(Barrier.GetGuid(), &Component);
}

UAGX_RigidBodyComponent* FAGX_SimObjectsImporterHelper::InstantiateBody(
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
			TEXT("AGX Dynamics RigidBody"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new AGX_RigidBodyComponent"));
		return nullptr;
	}

	TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateRigidBodyComponent(Barrier, *Component, Unused);
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
	return Component;
}

UAGX_SphereShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateSphere(
	const FSphereShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_SphereShapeComponent* Component = FAGX_EditorUtilities::CreateSphereShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Sphere"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new UAGX_SphereShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FSphereShapeBarrier& Barrier, UAGX_SphereShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	Component.CopyFrom(Barrier);
	UpdateShapeComponent(Barrier, Component, MSTsOnDisk);
}

UAGX_BoxShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateBox(
	const FBoxShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_BoxShapeComponent* Component = FAGX_EditorUtilities::CreateBoxShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Box"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new UAGX_BoxShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FBoxShapeBarrier& Barrier, UAGX_BoxShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	Component.CopyFrom(Barrier);
	UpdateShapeComponent(Barrier, Component, MSTsOnDisk);
}

UAGX_CylinderShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCylinder(
	const FCylinderShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_CylinderShapeComponent* Component =
		FAGX_EditorUtilities::CreateCylinderShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Cylinder"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new UAGX_CylinderShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FCylinderShapeBarrier& Barrier, UAGX_CylinderShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	Component.CopyFrom(Barrier);
	UpdateShapeComponent(Barrier, Component, MSTsOnDisk);
}

UAGX_CapsuleShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCapsule(
	const FCapsuleShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_CapsuleShapeComponent* Component = FAGX_EditorUtilities::CreateCapsuleShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Capsule"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new UAGX_CapsuleShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FCapsuleShapeBarrier& Barrier, UAGX_CapsuleShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	Component.CopyFrom(Barrier);
	UpdateShapeComponent(Barrier, Component, MSTsOnDisk);
}

UAGX_TrimeshShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateTrimesh(
	const FTrimeshShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_TrimeshShapeComponent* Component =
		FAGX_EditorUtilities::CreateTrimeshShape(&Owner, Body, false);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Trimesh"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not instantiate a new Trimesh Shape Component."));
		return nullptr;
	}

	Component->MeshSourceLocation = EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused);

	// Add collision Static mesh.
	UStaticMeshComponent* CollisionMesh =
		FAGX_ImportUtilities::CreateComponent<UStaticMeshComponent>(Owner, *Component);
	UpdateTrimeshCollisionMeshComponent(Barrier, *CollisionMesh);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *CollisionMesh);
	}

	Component->RegisterComponent();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateTrimeshCollisionMeshComponent(
	const FTrimeshShapeBarrier& ShapeBarrier, UStaticMeshComponent& Component)
{
	UStaticMesh* NewMeshAsset = nullptr;
	UStaticMesh* OriginalMeshAsset = Component.GetStaticMesh();

	if (IsMeshEquivalent(ShapeBarrier, OriginalMeshAsset))
	{
		NewMeshAsset = OriginalMeshAsset;
	}
	else
	{
		const FString FallbackName = ShapeBarrier.GetName().IsEmpty()
										 ? "CollisionMesh"
										 : ShapeBarrier.GetName() + FString("_CollisionMesh");
		UStaticMesh* Asset =
			GetOrCreateStaticMeshAsset(ShapeBarrier, FallbackName, RestoredMeshes, DirectoryName);
		NewMeshAsset = Asset;
	}

	FAGX_ImportUtilities::Rename(Component, *NewMeshAsset->GetName());

	UMaterialInterface* RenderMaterial = nullptr;
	if (ShapeBarrier.HasRenderData())
	{
		RenderMaterial = CreateRenderMaterialFromRenderDataOrDefault(
			ShapeBarrier.GetRenderData(), ShapeBarrier.GetIsSensor(), DirectoryName,
			RestoredRenderMaterials);
	}
	else
	{
		RenderMaterial = GetDefaultRenderMaterial(ShapeBarrier.GetIsSensor());
	}

	const bool Visible = !ShapeBarrier.HasValidRenderData();
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UStaticMeshComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Render Materials.
			if (Visible && Instance->GetMaterial(0) == Component.GetMaterial(0))
			{
				Instance->SetMaterial(0, RenderMaterial);
			}

			// Update Mesh asset.
			if (Instance->GetStaticMesh() == Component.GetStaticMesh())
			{
				Instance->SetStaticMesh(NewMeshAsset);
			}

			// Update visibility.
			if (Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetMaterial(0, RenderMaterial);
	Component.SetStaticMesh(NewMeshAsset);
	Component.SetVisibility(Visible);
	if (Visible)
		SetDefaultRenderMaterial(Component, ShapeBarrier.GetIsSensor());

	const FGuid ShapeGuid = ShapeBarrier.GetShapeGuid();
	if (!RestoredCollisionStaticMeshComponents.Contains(ShapeGuid))
	{
		RestoredCollisionStaticMeshComponents.Add(ShapeGuid, &Component);
	}
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FTrimeshShapeBarrier& Barrier, UAGX_TrimeshShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	Component.CopyFrom(Barrier);
	UpdateShapeComponent(Barrier, Component, MSTsOnDisk);
}

void FAGX_SimObjectsImporterHelper::UpdateShapeComponent(
	const FShapeBarrier& Barrier, UAGX_ShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk)
{
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());

	FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
	UAGX_ShapeMaterial* NewShapeMaterial = nullptr;
	if (NativeMaterial.HasNative())
	{
		const FGuid Guid = NativeMaterial.GetGuid();
		NewShapeMaterial = RestoredShapeMaterials.FindRef(Guid);
	}

	UMaterialInterface* RenderMaterial = nullptr;
	if (Barrier.HasRenderData())
	{
		RenderMaterial = CreateRenderMaterialFromRenderDataOrDefault(
			Barrier.GetRenderData(), Barrier.GetIsSensor(), DirectoryName, RestoredRenderMaterials);
	}
	else
	{
		RenderMaterial = GetDefaultRenderMaterial(Barrier.GetIsSensor());
	}

	const FMergeSplitThresholdsBarrier ThresholdsBarrier =
		FShapeContactMergeSplitThresholdsBarrier::CreateFrom(Barrier);
	const FGuid MSTGuid = Barrier.GetShapeGuid();
	UAGX_MergeSplitThresholdsBase* MSThresholds = nullptr;
	if (MSTsOnDisk.Contains(MSTGuid) && ThresholdsBarrier.HasNative())
	{
		MSThresholds = MSTsOnDisk[MSTGuid];
		::UpdateAndSaveMergeSplitThresholdsAsset(
			ThresholdsBarrier, *MSThresholds, RestoredThresholds, EAGX_AmorOwningType::BodyOrShape);
	}
	else
	{
		MSThresholds = ::GetOrCreateMergeSplitThresholdsAsset<
			FShapeBarrier, FShapeContactMergeSplitThresholdsBarrier>(
			Barrier, EAGX_AmorOwningType::BodyOrShape, RestoredThresholds, DirectoryName);
	}

	const bool Visible = !Barrier.HasRenderData() || (!Barrier.HasValidRenderData() &&
													  Barrier.GetRenderData().GetShouldRender());
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UAGX_ShapeComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			if (Instance->GetMaterial(0) == Component.GetMaterial(0))
			{
				Instance->SetMaterial(0, RenderMaterial);
			}

			Instance->UpdateVisualMesh();

			if (Instance->ShapeMaterial == Component.ShapeMaterial)
			{
				Instance->ShapeMaterial = NewShapeMaterial;
			}

			if (Instance->MergeSplitProperties.Thresholds ==
				Component.MergeSplitProperties.Thresholds)
			{
				Instance->MergeSplitProperties.Thresholds =
					Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
			}

			if (Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetMaterial(0, RenderMaterial);
	Component.UpdateVisualMesh();
	Component.ShapeMaterial = NewShapeMaterial;
	Component.MergeSplitProperties.Thresholds =
		Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
	Component.SetVisibility(Visible);
}

UStaticMeshComponent* FAGX_SimObjectsImporterHelper::InstantiateRenderData(
	const FShapeBarrier& ShapeBarrier, AActor& Owner, USceneComponent& AttachParent,
	FTransform* RelTransformOverride)
{
	const FRenderDataBarrier RenderDataBarrier = ShapeBarrier.GetRenderData();
	if (!RenderDataBarrier.HasMesh())
		return nullptr;

	UStaticMeshComponent* RenderMeshComponent =
		FAGX_ImportUtilities::CreateComponent<UStaticMeshComponent>(Owner, AttachParent);

	UpdateRenderDataComponent(
		ShapeBarrier, RenderDataBarrier, *RenderMeshComponent, RelTransformOverride);

	return RenderMeshComponent;
}

UStaticMeshComponent* FAGX_SimObjectsImporterHelper::InstantiateRenderDataInBodyOrRoot(
	const FTrimeshShapeBarrier& TrimeshBarrier, AActor& Owner, const FRigidBodyBarrier* Body)
{
	USceneComponent* AttachParent = [&]() -> USceneComponent*
	{
		if (Body != nullptr)
		{
			if (auto BodyComponent = GetBody(*Body))
			{
				return BodyComponent;
			}
		}

		return Owner.GetRootComponent();
	}();

	FTransform RenderDataTransform = FTransform::Identity;
	{
		FVector TrimeshPosition;
		FQuat TrimeshRotation;
		std::tie(TrimeshPosition, TrimeshRotation) = TrimeshBarrier.GetLocalPositionAndRotation();
		const FTransform TrimeshTransform(TrimeshRotation, TrimeshPosition);
		const FTransform ShapeToGeometry = TrimeshBarrier.GetGeometryToShapeTransform().Inverse();
		FTransform::Multiply(&RenderDataTransform, &ShapeToGeometry, &TrimeshTransform);
	}

	return InstantiateRenderData(TrimeshBarrier, Owner, *AttachParent, &RenderDataTransform);
}

void FAGX_SimObjectsImporterHelper::UpdateRenderDataComponent(
	const FShapeBarrier& ShapeBarrier, const FRenderDataBarrier& RenderDataBarrier,
	UStaticMeshComponent& Component, FTransform* RelTransformOverride)
{
	AGX_CHECK(RenderDataBarrier.HasMesh());

	// The triangles in the AGX Dynamics render data are relative to the Geometry, but the
	// Unreal Engine Component we create is placed at the position of the AGX Dynamics
	// Shape. There is no Component for the Geometry. To get the triangles in the right
	// place we need to offset the render data Component by the inverse of the
	// Geometry-to-Shape transformation in the source AGX Dynamics data.
	//
	// In AGX Dynamics:
	//
	//   Geometry           Shape       A
	//   origin             origin   triangle
	//     X                  O         |
	//     '-----------28-------------->'
	//           The vertex position
	//     '--------18------->'
	//      The Shape position
	//      relative to the
	//      geometry
	//
	//
	// In Unreal Engine:
	//
	//                      Shape       A
	//                      origin   triangle
	//                        O         |                  |
	//                        '-------------28------------>'
	//                         Where the triangle would end
	//                         up if the vertex position is
	//                         used as-is.
	//                                  '<------18---------'
	//                                   The inverse of the
	//                                   Geometry-to-Shape
	//                                   transformation.
	FTransform NewRelTransform = RelTransformOverride != nullptr
									 ? *RelTransformOverride
									 : ShapeBarrier.GetGeometryToShapeTransform().Inverse();

	UMaterialInterface* OriginalRenderMaterial = Component.GetMaterial(0);

	// We always create a new Render Material because it is not trivial to determine if the original
	// render material corresponds to the data in the RenderDataBarrier.
	UMaterialInterface* NewRenderMaterial = CreateRenderMaterialFromRenderDataOrDefault(
		RenderDataBarrier, ShapeBarrier.GetIsSensor(), DirectoryName, RestoredRenderMaterials);
	UStaticMesh* NewMeshAsset = nullptr;

	UStaticMesh* OriginalMeshAsset = Component.GetStaticMesh();
	if (IsMeshEquivalent(RenderDataBarrier, OriginalMeshAsset))
	{
		NewMeshAsset = OriginalMeshAsset;
	}
	else
	{
		UStaticMesh* Asset =
			GetOrCreateStaticMeshAsset(RenderDataBarrier, RestoredMeshes, DirectoryName);
		NewMeshAsset = Asset;
	}

	FAGX_ImportUtilities::Rename(Component, *NewMeshAsset->GetName());

	const bool Visible = RenderDataBarrier.GetShouldRender();
	FAGX_BlueprintUtilities::SetTemplateComponentRelativeTransform(Component, NewRelTransform);

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UStaticMeshComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Render Materials.
			if (Instance->GetMaterial(0) == OriginalRenderMaterial)
			{
				Instance->SetMaterial(0, NewRenderMaterial);
			}

			// Update Mesh asset.
			if (Instance->GetStaticMesh() == Component.GetStaticMesh())
			{
				Instance->SetStaticMesh(NewMeshAsset);
			}

			// Update visibility.
			if (Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetMaterial(0, NewRenderMaterial);
	Component.SetStaticMesh(NewMeshAsset);
	Component.SetVisibility(Visible);

	const FGuid RenderDataGuid = RenderDataBarrier.GetGuid();
	if (!RestoredRenderStaticMeshComponents.Contains(RenderDataGuid))
	{
		RestoredRenderStaticMeshComponents.Add(RenderDataGuid, &Component);
	}
}

void FAGX_SimObjectsImporterHelper::UpdateAndSaveShapeMaterialAsset(
	const FShapeMaterialBarrier& Barrier, UAGX_ShapeMaterial& Asset)
{
	Asset.CopyFrom(&Barrier);
	FAGX_EditorUtilities::RenameAsset(Asset, Barrier.GetName(), "ShapeMaterial");
	FAGX_ObjectUtilities::SaveAsset(Asset);

	RestoredShapeMaterials.Add(Barrier.GetGuid(), &Asset);
}

UAGX_ShapeMaterial* FAGX_SimObjectsImporterHelper::InstantiateShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	UAGX_ShapeMaterial* Asset = FAGX_ImportUtilities::CreateAsset<UAGX_ShapeMaterial>(
		DirectoryName, Barrier.GetName(),
		FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName());
	if (Asset == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Shape Material"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create a Shape Material Asset from given ShapeMaterialBarrier."));
		return nullptr;
	}

	UpdateAndSaveShapeMaterialAsset(Barrier, *Asset);
	return Asset;
}

void FAGX_SimObjectsImporterHelper::UpdateAndSaveContactMaterialAsset(
	const FContactMaterialBarrier& Barrier, UAGX_ContactMaterial& Asset,
	UAGX_ContactMaterialRegistrarComponent& CMRegistrar)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	Asset.CopyFrom(Barrier);
	Asset.Material1 = Materials.first;
	Asset.Material2 = Materials.second;

	const FString Name = TEXT("CM_") + GetName(Materials.first) + GetName(Materials.second);
	FAGX_EditorUtilities::RenameAsset(Asset, Name, "ContactMaterial");
	FAGX_ObjectUtilities::SaveAsset(Asset);

	CMRegistrar.ContactMaterials.AddUnique(&Asset);
	if (FAGX_ObjectUtilities::IsTemplateComponent(CMRegistrar))
	{
		for (UAGX_ContactMaterialRegistrarComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(CMRegistrar))
		{
			Instance->ContactMaterials.AddUnique(&Asset);
		}
	}
}

UAGX_ContactMaterial* FAGX_SimObjectsImporterHelper::InstantiateContactMaterial(
	const FContactMaterialBarrier& Barrier, UAGX_ContactMaterialRegistrarComponent& CMRegistrar)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	const FString Name = TEXT("CM_") + GetName(Materials.first) + GetName(Materials.second);
	UAGX_ContactMaterial* Asset = FAGX_ImportUtilities::CreateAsset<UAGX_ContactMaterial>(
		DirectoryName, Name, FAGX_ImportUtilities::GetImportContactMaterialDirectoryName());
	if (Asset == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Contact Material"), Name, ImportSettings.FilePath,
			TEXT("Could not create a Contact Material Asset from given ContactMaterialBarrier."));
		return nullptr;
	}

	UpdateAndSaveContactMaterialAsset(Barrier, *Asset, CMRegistrar);
	return Asset;
}

UAGX_ContactMaterialRegistrarComponent*
FAGX_SimObjectsImporterHelper::InstantiateContactMaterialRegistrar(AActor& Owner)
{
	UAGX_ContactMaterialRegistrarComponent* Component =
		NewObject<UAGX_ContactMaterialRegistrarComponent>(
			&Owner, TEXT("AGX_ContactMaterialRegistrar"));

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

namespace
{
	// This function does not update Constraint Controllers and should not be called in isolation
	// for 1 or 2 DOF Constraints. Instead, one of the UpdateConstraintNDofComponent functions
	// should be called to completely update those.
	void UpdateConstraintComponentNoControllers(
		UAGX_ConstraintComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& RestoredThresholds)
	{
		FAGX_SimObjectsImporterHelper::FBodyPair Bodies = Helper.GetBodies(Barrier);
		if (Bodies.first == nullptr)
		{
			// Not having a second body is fine, means that the first body is constrained to the
			// world. Not having a first body is unexpected.
			UE_LOG(
				LogAGX, Warning,
				TEXT("Constraint '%s' imported from '%s' does not have a first body."),
				*Barrier.GetName(), *Helper.ImportSettings.FilePath);
		}

		auto ToFName = [](const UAGX_RigidBodyComponent& Body)
		{
			if (FAGX_ObjectUtilities::IsTemplateComponent(Body))
			{
				return FName(FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
					Body.GetName()));
			}
			return Body.GetFName();
		};

		const TOptional<FName> BodyName1 =
			Bodies.first != nullptr ? ToFName(*Bodies.first) : TOptional<FName>();
		const TOptional<FName> BodyName2 =
			Bodies.second != nullptr ? ToFName(*Bodies.second) : TOptional<FName>();

		// Update Body Name of the attachments.
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Constraint))
		{
			if (Instance->BodyAttachment1.RigidBody.BodyName ==
					Constraint.BodyAttachment1.RigidBody.BodyName &&
				BodyName1.IsSet())
			{
				Instance->BodyAttachment1.RigidBody.BodyName = BodyName1.GetValue();
			}

			if (Instance->BodyAttachment2.RigidBody.BodyName ==
					Constraint.BodyAttachment2.RigidBody.BodyName &&
				BodyName2.IsSet())
			{
				Instance->BodyAttachment2.RigidBody.BodyName = BodyName2.GetValue();
			}
		}

		if (BodyName1.IsSet())
		{
			Constraint.BodyAttachment1.RigidBody.BodyName = BodyName1.GetValue();
		}
		if (BodyName2.IsSet())
		{
			Constraint.BodyAttachment2.RigidBody.BodyName = BodyName2.GetValue();
		}

		FAGX_ImportUtilities::Rename(Constraint, Barrier.GetName());
		Constraint.CopyFrom(Barrier);
		const FTransform NewWorldTransform =
			FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
				Barrier, Constraint, Bodies.first, Bodies.second);
		if (FAGX_ObjectUtilities::IsTemplateComponent(Constraint))
		{
			FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
				&Constraint, NewWorldTransform);
		}

		if (auto ThresholdsAsset = ::GetOrCreateMergeSplitThresholdsAsset<
				FConstraintBarrier, FConstraintMergeSplitThresholdsBarrier>(
				Barrier, EAGX_AmorOwningType::Constraint, RestoredThresholds, Helper.DirectoryName))
		{
			Constraint.MergeSplitProperties.Thresholds =
				Cast<UAGX_ConstraintMergeSplitThresholds>(ThresholdsAsset);
		}
	}

	void UpdateConstraint1DofComponent(
		UAGX_Constraint1DofComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& RestoredThresholds)
	{
		UpdateConstraintComponentNoControllers(Constraint, Barrier, Helper, RestoredThresholds);
		FAGX_ConstraintUtilities::CopyControllersFrom(
			Constraint, *static_cast<const FConstraint1DOFBarrier*>(&Barrier));
	}

	void UpdateConstraint2DofComponent(
		UAGX_Constraint2DofComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& RestoredThresholds)
	{
		UpdateConstraintComponentNoControllers(Constraint, Barrier, Helper, RestoredThresholds);
		FAGX_ConstraintUtilities::CopyControllersFrom(
			Constraint, *static_cast<const FConstraint2DOFBarrier*>(&Barrier));
	}
}

UAGX_HingeConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier, AActor& Owner)
{
	UAGX_HingeConstraintComponent* Constraint = NewObject<UAGX_HingeConstraintComponent>(&Owner);
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_PrismaticConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier, AActor& Owner)
{
	UAGX_PrismaticConstraintComponent* Constraint =
		NewObject<UAGX_PrismaticConstraintComponent>(&Owner);
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_BallConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateBallConstraint(
	const FBallJointBarrier& Barrier, AActor& Owner)
{
	UAGX_BallConstraintComponent* Constraint = NewObject<UAGX_BallConstraintComponent>(&Owner);
	UpdateConstraintComponentNoControllers(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_CylindricalConstraintComponent*
FAGX_SimObjectsImporterHelper::InstantiateCylindricalConstraint(
	const FCylindricalJointBarrier& Barrier, AActor& Owner)
{
	UAGX_CylindricalConstraintComponent* Constraint =
		NewObject<UAGX_CylindricalConstraintComponent>(&Owner);
	UpdateConstraint2DofComponent(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_DistanceConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateDistanceConstraint(
	const FDistanceJointBarrier& Barrier, AActor& Owner)
{
	UAGX_DistanceConstraintComponent* Constraint =
		NewObject<UAGX_DistanceConstraintComponent>(&Owner);
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_LockConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateLockConstraint(
	const FLockJointBarrier& Barrier, AActor& Owner)
{
	UAGX_LockConstraintComponent* Constraint = NewObject<UAGX_LockConstraintComponent>(&Owner);
	UpdateConstraintComponentNoControllers(*Constraint, Barrier, *this, RestoredThresholds);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

void FAGX_SimObjectsImporterHelper::UpdateConstraintComponent(
	const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component)
{
	if (UAGX_Constraint1DofComponent* Constraint1Dof =
			Cast<UAGX_Constraint1DofComponent>(&Component))
	{
		UpdateConstraint1DofComponent(*Constraint1Dof, Barrier, *this, RestoredThresholds);
	}
	else if (
		UAGX_Constraint2DofComponent* Constraint2Dof =
			Cast<UAGX_Constraint2DofComponent>(&Component))
	{
		UpdateConstraint2DofComponent(*Constraint2Dof, Barrier, *this, RestoredThresholds);
	}
	else
	{
		UpdateConstraintComponentNoControllers(Component, Barrier, *this, RestoredThresholds);
	}
}

UAGX_TwoBodyTireComponent* FAGX_SimObjectsImporterHelper::InstantiateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, AActor& Owner)
{
	UAGX_TwoBodyTireComponent* Component = NewObject<UAGX_TwoBodyTireComponent>(&Owner);

	UpdateTwoBodyTire(Barrier, *Component);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, UAGX_TwoBodyTireComponent& Component)
{
	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef)
	{
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), ImportSettings.FilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.BodyName = Body->GetFName();
	};

	SetRigidBody(GetBody(Barrier.GetTireRigidBody()), Component.TireRigidBody);
	SetRigidBody(GetBody(Barrier.GetHubRigidBody()), Component.HubRigidBody);
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());
	Component.CopyFrom(Barrier);
}

UAGX_CollisionGroupDisablerComponent*
FAGX_SimObjectsImporterHelper::InstantiateCollisionGroupDisabler(
	AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs)
{
	UAGX_CollisionGroupDisablerComponent* Component =
		NewObject<UAGX_CollisionGroupDisablerComponent>(&Owner);

	UpdateCollisionGroupDisabler(DisabledPairs, *Component);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateCollisionGroupDisabler(
	const TArray<std::pair<FString, FString>>& DisabledPairs,
	UAGX_CollisionGroupDisablerComponent& Component)
{
	FAGX_ImportUtilities::Rename(Component, TEXT("AGX_CollisionGroupDisabler"));

	// Update any archetype instance with the new groups.
	for (UAGX_CollisionGroupDisablerComponent* Instance :
		 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		Instance->UpdateAvailableCollisionGroupsFromWorld();
		Instance->RemoveDeprecatedCollisionGroups();

		for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
		{
			Instance->DisableCollisionGroupPair(
				FName(*DisabledPair.first), FName(*DisabledPair.second), true);
		}
	}

	Component.UpdateAvailableCollisionGroupsFromWorld();
	Component.RemoveDeprecatedCollisionGroups();
	for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
	{
		Component.DisableCollisionGroupPair(
			FName(*DisabledPair.first), FName(*DisabledPair.second), true);
	}
}

UAGX_WireComponent* FAGX_SimObjectsImporterHelper::InstantiateWire(
	const FWireBarrier& Barrier, AActor& Owner)
{
	UAGX_WireComponent* Component = NewObject<UAGX_WireComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Wire"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new AGX_WireComponent"));
		return nullptr;
	}

	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());

	// Copy simple properties such as radius and segment length. More complicated properties, such
	// as physical material, winches and route nodes, are handled below.
	Component->CopyFrom(Barrier);

	// Find and assign the physical material asset.
	FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
	if (NativeMaterial.HasNative())
	{
		const FGuid Guid = NativeMaterial.GetGuid();
		UAGX_ShapeMaterial* Material = RestoredShapeMaterials.FindRef(Guid);
		Component->ShapeMaterial = Material;
	}

	// Configure winches.
	auto ConfigureWinch = [this, &Barrier](EWireSide Side, UAGX_WireComponent& Wire) -> bool
	{
		FWireWinchBarrier WinchBarrier = Barrier.GetWinch(Side);
		if (WinchBarrier.HasNative())
		{
			FAGX_WireWinch* Winch = Wire.GetOwnedWinch(Side);
			// Get Owned Winch can never return nullptr when a valid Side is passed.
			Winch->CopyFrom(WinchBarrier);
			FRigidBodyBarrier WinchBodyBarrier = WinchBarrier.GetRigidBody();
			UAGX_RigidBodyComponent* WinchBodyComponent = GetBody(WinchBodyBarrier);
			// Ok for WinchBodyComponent to be nullptr. Means attached to the world.
			Winch->SetBodyAttachment(WinchBodyComponent);
			Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::Wire);
			return true;
		}
		else
		{
			Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::None);
			return false;
		}
	};
	bool bHaveBeginWinch = ConfigureWinch(EWireSide::Begin, *Component);
	bool bHaveEndWinch = ConfigureWinch(EWireSide::End, *Component);

	TArray<FWireRoutingNode>& Route = Component->RouteNodes;
	Route.Empty();

	// Helper function to create Body Fixed nodes.
	auto TryCreateBodyFixedNode = [this, &Route](FWireNodeBarrier NodeBarrier)
	{
		if (NodeBarrier.GetType() != EWireNodeType::BodyFixed)
		{
			return;
		}
		FRigidBodyBarrier NodeBodyBarrier = NodeBarrier.GetRigidBody();
		if (!NodeBodyBarrier.HasNative())
		{
			/// @todo Is it OK to have a Body Fixed Node without a body?
			return;
		}
		UAGX_RigidBodyComponent* Body = GetBody(NodeBodyBarrier);
		if (Body == nullptr)
		{
			/// @todo Is it OK to have a Body Fixed Node without a body?
			return;
		}

		if (Route.Num() > 0 && Route[0].NodeType == EWireNodeType::Free)
		{
			// In an initialized wire there may be a Free node right on top of the Body Fixe node.
			// The Body Fixed node represents the body itself, while the Free node represents the
			// part of the wire that approaches the body. While routing we only need the Body Fixed
			// node, it represents both concepts.
			Route.Pop();
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = EWireNodeType::BodyFixed;
		/// @todo This should be changed to GetLocation once route nodes with a body have their
		/// location relative to the body instead of the wire.
		RouteNode.Location = NodeBarrier.GetWorldLocation();
		RouteNode.SetBody(Body);
		Route.Add(RouteNode);
	};

	if (!bHaveBeginWinch)
	{
		// Configure initial Body Fixe node. Some Body Fixed nodes are owned by the winch on that
		// side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetFirstNode());
	}

	// Configure "normal" route nodes.
	for (auto It = Barrier.GetRenderBeginIterator(), End = Barrier.GetRenderEndIterator();
		 It != End; It.Inc())
	{
		const FAGX_WireNode NodeAGX = It.Get();
		const EWireNodeType NodeType = [&NodeAGX, &Barrier, &It]() -> EWireNodeType
		{
			if (Barrier.IsLumpedNode(It))
			{
				// Lumped nodes are special somehow, and should be created as free nodes.
				return EWireNodeType::Free;
			}
			switch (NodeAGX.GetType())
			{
				case EWireNodeType::Free:
					// Free nodes can be routed as-is.
					return EWireNodeType::Free;
				case EWireNodeType::Eye:
					// Eye nodes can be routed as-is.
					return EWireNodeType::Eye;
				case EWireNodeType::BodyFixed:
					// A Body Fixed node found by the render iterator is an implicitly created node
					// and should be routed as a Free node. It should not be attached to any of the
					// Rigid Bodies in the Component list.
					return EWireNodeType::Free;
				case EWireNodeType::Stop:
					// Stop nodes are used by winches, which we detect with GetWinch instead.
					return EWireNodeType::Other;
				default:
					// Any other node type is routed as a Free node for now. Special handling may
					// be needed in the future, if routing with additional node types become
					// supported.
					return EWireNodeType::Free;
			}
		}();

		if (NodeType == EWireNodeType::Other)
		{
			// Other nodes are used to signal "skip this node".
			continue;
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = NodeType;
		/// @todo This should be changed to GetLocation for nodes with a body once route nodes with
		/// a body have their location relative to the body instead of the wire.
		RouteNode.Location = NodeAGX.GetWorldLocation();

		if (NodeType == EWireNodeType::Eye)
		{
			FRigidBodyBarrier BodyBarrier = NodeAGX.GetRigidBody();
			UAGX_RigidBodyComponent* BodyComponent = GetBody(BodyBarrier);
			if (BodyComponent != nullptr)
			{
				RouteNode.SetBody(BodyComponent);
			}
		}

		Route.Add(RouteNode);
	}

	if (!bHaveEndWinch)
	{
		// Configure ending Body Fixed node. Some Body Fixed nodes are owned by the winch on that
		//	side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetLastNode());
	}

	if (auto ThresholdsAsset =
			::GetOrCreateMergeSplitThresholdsAsset<FWireBarrier, FWireMergeSplitThresholdsBarrier>(
				Barrier, EAGX_AmorOwningType::Wire, RestoredThresholds, DirectoryName))
	{
		Component->MergeSplitProperties.Thresholds =
			Cast<UAGX_WireMergeSplitThresholds>(ThresholdsAsset);
	}

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();
	// May chose to store a table of all imported wires. If so, add this wire to the table here.
	return Component;
}

UAGX_TrackComponent* FAGX_SimObjectsImporterHelper::InstantiateTrack(
	const FTrackBarrier& Barrier, AActor& Owner)
{
	UAGX_TrackComponent* Component = NewObject<UAGX_TrackComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Track"), Barrier.GetName(), ImportSettings.FilePath,
			TEXT("Could not create new AGX_TrackComponent"));
		return nullptr;
	}

	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());

	// Copy simple properties such as number of nodes and width. More complicated properties, such
	// as Wheels, TrackProperties asset etc, are handled below.
	Component->CopyFrom(Barrier);

	// Apply Shape Material.
	FShapeMaterialBarrier ShapeMaterial = Barrier.GetMaterial();
	if (ShapeMaterial.HasNative())
	{
		const FGuid Guid = ShapeMaterial.GetGuid();
		UAGX_ShapeMaterial* Material = RestoredShapeMaterials.FindRef(Guid);
		Component->ShapeMaterial = Material;
	}

	const FString BarrierName = Barrier.GetName();

	// Apply Track Properties.
	{
		const FString AssetName =
			BarrierName.IsEmpty() ? FString("AGX_TP_Track") : FString("AGX_TP_") + BarrierName;

		UAGX_TrackProperties* TrackProperties = GetOrCreateTrackPropertiesAsset(
			Barrier.GetProperties(), AssetName, RestoredTrackProperties, DirectoryName);
		if (TrackProperties == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unable to create an Asset for the Track Properties '%s' of Track '%s' during "
					 "import."),
				*AssetName, *Barrier.GetName());
		}
		else
		{
			Component->TrackProperties = TrackProperties;
		}
	}

	// Apply Internal Merge Properties.
	{
		const FString AssetName =
			BarrierName.IsEmpty() ? FString("AGX_TIMP_Track") : FString("AGX_TIMP_") + BarrierName;

		Component->InternalMergeProperties =
			FAGX_ImportUtilities::SaveImportedTrackInternalMergePropertiesAsset(
				Barrier, DirectoryName, AssetName);
	}

	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef)
	{
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics Track"), Barrier.GetName(), ImportSettings.FilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.BodyName = Body->GetFName();
	};

	// Copy Wheels.
	for (const FTrackWheelBarrier& WheelBarrier : Barrier.GetWheels())
	{
		FAGX_TrackWheel Wheel;
		SetRigidBody(GetBody(WheelBarrier.GetRigidBody()), Wheel.RigidBody);
		Wheel.bUseFrameDefiningComponent = false;
		Wheel.RelativeLocation = WheelBarrier.GetRelativeLocation();
		Wheel.RelativeRotation = WheelBarrier.GetRelativeRotation();
		Wheel.Radius = static_cast<float>(WheelBarrier.GetRadius());
		Wheel.Model = WheelBarrier.GetModel();
		Wheel.bSplitSegments = WheelBarrier.GetSplitSegments();
		Wheel.bMoveNodesToRotationPlane = WheelBarrier.GetMoveNodesToRotationPlane();
		Wheel.bMoveNodesToWheel = WheelBarrier.GetMoveNodesToWheel();
		Component->Wheels.Add(Wheel);
	}

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();
	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateReImportComponent(UAGX_ReImportComponent& Component)
{
	auto UpdateReImportComponent = [this](UAGX_ReImportComponent* C)
	{
		if (C == nullptr)
		{
			return;
		}

		C->FilePath = ImportSettings.FilePath;
		C->bIgnoreDisabledTrimeshes = ImportSettings.bIgnoreDisabledTrimeshes;
		C->StaticMeshComponentToOwningTrimesh.Empty();
		for (const auto& RestoredSMCTuple : RestoredCollisionStaticMeshComponents)
		{
			const FString Name = RestoredSMCTuple.Value->GetName();
			AGX_CHECK(!C->StaticMeshComponentToOwningTrimesh.Contains(Name));
			C->StaticMeshComponentToOwningTrimesh.Add(Name, RestoredSMCTuple.Key);
		}

		C->StaticMeshComponentToOwningRenderData.Empty();
		for (const auto& RestoredSMCTuple : RestoredRenderStaticMeshComponents)
		{
			const FString Name = RestoredSMCTuple.Value->GetName();
			AGX_CHECK(!C->StaticMeshComponentToOwningTrimesh.Contains(Name));
			C->StaticMeshComponentToOwningRenderData.Add(Name, RestoredSMCTuple.Key);
		}
	};

	for (UAGX_ReImportComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		UpdateReImportComponent(Instance);
	}

	UpdateReImportComponent(&Component);
	FAGX_ImportUtilities::Rename(Component, "AGX_ReImport");
}

UAGX_ReImportComponent* FAGX_SimObjectsImporterHelper::InstantiateReImportComponent(AActor& Owner)
{
	UAGX_ReImportComponent* ReImportComponent = NewObject<UAGX_ReImportComponent>(&Owner);
	if (ReImportComponent == nullptr)
	{
		return nullptr;
	}

	UpdateReImportComponent(*ReImportComponent);
	ReImportComponent->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(ReImportComponent);
	ReImportComponent->RegisterComponent();
	ReImportComponent->PostEditChange();

	return ReImportComponent;
}

UAGX_ObserverFrameComponent* FAGX_SimObjectsImporterHelper::InstantiateObserverFrame(
	const FString& Name, const FGuid& BodyGuid, const FGuid& ObserverGuid,
	const FTransform& Transform, AActor& Owner)
{
	// Get the Rigid Body the imported Observer Frame should be attached to.
	UAGX_RigidBodyComponent* Body = RestoredBodies.FindRef(BodyGuid);
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("While importing from '%s': Observer Frame %s is attached to a Rigid Body that "
				 "has not been restored. Cannot create Unreal Engine representation. Tried to find "
				 "Rigid Body with GUID %s."),
			*ImportSettings.FilePath, *Name, *BodyGuid.ToString());
		return nullptr;
	}

	UAGX_ObserverFrameComponent* Component =
		FAGX_ImportUtilities::CreateComponent<UAGX_ObserverFrameComponent>(Owner, *Body);

	UpdateObserverFrameComponent(Name, ObserverGuid, Transform, *Component);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateObserverFrameComponent(
	const FString& Name, const FGuid& ObserverGuid, const FTransform& Transform,
	UAGX_ObserverFrameComponent& Component)
{
	FAGX_ImportUtilities::Rename(Component, Name);
	FAGX_BlueprintUtilities::SetTemplateComponentRelativeTransform(Component, Transform);

	// Update any archetype instances.
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (UAGX_ObserverFrameComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Import Guid.
			Instance->ImportGuid = ObserverGuid;
		}
	}

	Component.ImportGuid = ObserverGuid;
}

UAGX_RigidBodyComponent* FAGX_SimObjectsImporterHelper::GetBody(
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
			*ImportSettings.FilePath, *Barrier.GetName());
	}

	return Component;
}

FAGX_SimObjectsImporterHelper::FBodyPair FAGX_SimObjectsImporterHelper::GetBodies(
	const FConstraintBarrier& Barrier)
{
	return {GetBody(Barrier.GetFirstBody()), GetBody(Barrier.GetSecondBody())};
}

UAGX_ShapeMaterial* FAGX_SimObjectsImporterHelper::GetShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	return RestoredShapeMaterials.FindRef(Barrier.GetGuid());
}

FAGX_SimObjectsImporterHelper::FShapeMaterialPair FAGX_SimObjectsImporterHelper::GetShapeMaterials(
	const FContactMaterialBarrier& ContactMaterial)
{
	return {
		GetShapeMaterial(ContactMaterial.GetMaterial1()),
		GetShapeMaterial(ContactMaterial.GetMaterial2())};
}

void FAGX_SimObjectsImporterHelper::FinalizeImport()
{
	// Build mesh assets.
	TArray<UStaticMesh*> StaticMeshes;
	RestoredMeshes.GenerateValueArray(StaticMeshes);
	FAGX_EditorUtilities::SaveStaticMeshAssetsInBulk(StaticMeshes);
}

namespace
{
	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(
			SourceFilename, FAGX_ImportUtilities::GetImportRootDirectoryName());
	}

	FString MakeDirectoryName(const FString ModelName)
	{
		FString BasePath = FAGX_ImportUtilities::CreatePackagePath(ModelName);

		auto PackageExists = [&](const FString& DirPath)
		{
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
		FString DirectoryName = ModelName;
		while (PackageExists(DirectoryPath))
		{
			++TryCount;
			DirectoryPath = BasePath + TEXT("_") + FString::FromInt(TryCount);
			DirectoryName = ModelName + TEXT("_") + FString::FromInt(TryCount);
		}
		UE_LOG(LogAGX, Display, TEXT("Importing model '%s' to '%s'."), *ModelName, *DirectoryPath);
		return DirectoryName;
	}
}

FAGX_SimObjectsImporterHelper::FAGX_SimObjectsImporterHelper(
	const FAGX_ImportSettings& InImportSettings)
	: ImportSettings(InImportSettings)
	, SourceFileName(FPaths::GetBaseFilename(InImportSettings.FilePath))
	, DirectoryName(MakeDirectoryName(MakeModelName(SourceFileName)))
{
}

FAGX_SimObjectsImporterHelper::FAGX_SimObjectsImporterHelper(
	const FAGX_ImportSettings& InImportSettings, const FString& InDirectoryName)
	: ImportSettings(InImportSettings)
	, SourceFileName(FPaths::GetBaseFilename(InImportSettings.FilePath))
	, DirectoryName(InDirectoryName)
{
}
