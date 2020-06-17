#pragma once

// AGXUnreal includes.
#include "AGXArchiveReader.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Misc/Guid.h"

// AGXUnreal classes.
class AAGX_RigidBodyActor;
class UAGX_RigidBodyComponent;
class UAGX_SphereShapeComponent;
class UAGX_BoxShapeComponent;
class UAGX_CylinderShapeComponent;
class UAGX_TrimeshShapeComponent;
class UAGX_ShapeMaterialAsset;
class UAGX_ContactMaterialAsset;

// Unreal Engine classes.
class AActor;
class UStaticMesh;

/**
 * An Unreal Engine side helper that creates `[UA]AGX_.*` objects from Barrier objects read from an
 * AGX Dynamics archive.
 */
struct FAGX_ArchiveImporterHelper
{
public:
	/** Create a new UAGX_RigidBodyComponent in the given actor. */
	UAGX_RigidBodyComponent* InstantiateBody(const FRigidBodyBarrier& Barrier, AActor& Actor);

	/** Create a new AAGX_RigidBodyActor in the given world. */
	AAGX_RigidBodyActor* InstantiateBody(const FRigidBodyBarrier& Barrier, UWorld& World);

	UAGX_SphereShapeComponent* InstantiateSphere(
		const FSphereShapeBarrier& Sphere, UAGX_RigidBodyComponent& Body);

	UAGX_BoxShapeComponent* InstantiateBox(
		const FBoxShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body);

	UAGX_CylinderShapeComponent* InstantiateCylinder(
		const FCylinderShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body);

	UAGX_TrimeshShapeComponent* InstantiateTrimesh(
		const FTrimeshShapeBarrier& Barrier, UAGX_RigidBodyComponent& Body);

	UAGX_ShapeMaterialAsset* InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier);

	UAGX_ContactMaterialAsset* InstantiateContactMaterial(const FContactMaterialBarrier& Barrier);

	UAGX_RigidBodyComponent* GetBody(const FRigidBodyBarrier& Barrier);

	using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;
	FBodyPair GetBodies(/*Constraint barrier here.*/);

	UAGX_ShapeMaterialAsset* GetShapeMaterial(const FShapeMaterialBarrier& Barrier);

	using FShapeMaterialPair = std::pair<UAGX_ShapeMaterialAsset*, UAGX_ShapeMaterialAsset*>;
	FShapeMaterialPair GetShapeMaterials(const FContactMaterialBarrier& ContactMaterial);

	explicit FAGX_ArchiveImporterHelper(const FString& ArchiveFilePath);

	const FString ArchiveFilePath;
	const FString ArchiveFileName;
	const FString ArchiveName;
	TMap<FGuid, UStaticMesh*> RestoredMeshes;
	TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
	TMap<FGuid, UAGX_ShapeMaterialAsset*> RestoredShapeMaterials;
};

/**
 * An ArchiveBody that creates nothing. Used when the Unreal object couldn't be created.
 */
class NopEditorBody final : public FAGXArchiveBody
{
	virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
	{
	}
};
