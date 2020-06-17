#include "AGX_ArchiveImporterHelper.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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
	NewActor->RigidBodyComponent->CopyFrom(Barrier);
	RestoredBodies.Add(Barrier.GetGuid(), NewActor->RigidBodyComponent);
	/// \todo Do we need to do any additional configuration here?
	return NewActor;
}

namespace
{
	void FinalizeShape(
		UAGX_ShapeComponent& Component, const FShapeBarrier& Barrier,
		const TMap<FGuid, UAGX_ShapeMaterialAsset*>& RestoredShapeMaterials)
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
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials);
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
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials);
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
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials);
	return Component;
}

namespace
{
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Barrier, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*> RestoredMeshes, const FString& ArchiveName)
	{
		FGuid Guid = Barrier.GetMeshDataGuid();

		// If the GUID is invalid, try to create the mesh asset anyway but without adding it to the
		// RestoredMeshes map.
		if (!Guid.IsValid())
		{
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Barrier, ArchiveName, FallbackName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			return Asset;
		}

		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(Barrier, ArchiveName, FallbackName);
		RestoredMeshes.Add(Guid, Asset);
		return Asset;
	}
}

UAGX_TrimeshShapeComponent* FAGX_ArchiveImporterHelper::InstantiateTrimesh(
	const FTrimeshShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body)
{
	AActor* Owner = Body.GetOwner();
	UAGX_TrimeshShapeComponent* Component = FAGX_EditorUtilities::CreateTrimeshShape(Owner, &Body);
	Component->MeshSourceLocation = EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
	UStaticMesh* MeshAsset =
		GetOrCreateStaticMeshAsset(Barrier, Body.GetName(), RestoredMeshes, ArchiveName);
	if (MeshAsset == nullptr)
	{
		// No point in continuing further. Logging handled in GetOrCreateStaticMeshAsset.
		/// \todo Consider moving logging in here, using WriteImportErrorMessage.
		return nullptr;
	}

	UStaticMeshComponent* MeshComponent =
		FAGX_EditorUtilities::CreateStaticMeshComponent(Owner, Component, MeshAsset);
	FAGX_ImportUtilities::Rename(*MeshComponent, *(Barrier.GetName() + TEXT("Mesh")));

	Component->CopyFrom(Barrier);
	::FinalizeShape(*Component, Barrier, RestoredShapeMaterials);
	return Component;
}

UAGX_ShapeMaterialAsset* FAGX_ArchiveImporterHelper::InstantiateShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	/// \todo Do we need any special handling of the default material?
	UAGX_ShapeMaterialAsset* Asset =
		FAGX_ImportUtilities::SaveImportedShapeMaterialAsset(Barrier, ArchiveName);
	RestoredShapeMaterials.Add(Barrier.GetGuid(), Asset);
	return Asset;
}

UAGX_ContactMaterialAsset* FAGX_ArchiveImporterHelper::InstantiateContactMaterial(
	const FContactMaterialBarrier& Barrier)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	UAGX_ContactMaterialAsset* Asset = FAGX_ImportUtilities::SaveImportedContactMaterialAsset(
		Barrier, Materials.first, Materials.second, ArchiveName);
	return Asset;
}

UAGX_RigidBodyComponent* FAGX_ArchiveImporterHelper::GetBody(const FRigidBodyBarrier& Barrier)
{
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
			LogAGX, Warning,
			TEXT("Found a constraint to body '%s', but that body hans't been restored."),
			*Barrier.GetName());
		return nullptr;
	}

	return Component;
}

FAGX_ArchiveImporterHelper::FBodyPair FAGX_ArchiveImporterHelper::GetBodies()
{
	/// \todo Continue here.
	return {nullptr, nullptr};
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
}

FAGX_ArchiveImporterHelper::FAGX_ArchiveImporterHelper(const FString& InArchiveFilePath)
	: ArchiveFilePath(InArchiveFilePath)
	, ArchiveFileName(FPaths::GetBaseFilename(InArchiveFilePath))
	, ArchiveName(MakeArchiveName(ArchiveFileName))
{
}
