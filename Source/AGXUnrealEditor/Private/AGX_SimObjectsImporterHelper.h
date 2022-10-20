// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ImportEnums.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Misc/Guid.h"

// AGXUnreal classes.
class FBallJointBarrier;
class FContactMaterialBarrier;
class FConstraintBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FHingeBarrier;
class FLockJointBarrier;
class FPrismaticBarrier;
class FShapeMaterialBarrier;
class FTwoBodyTireBarrier;
class FWireBarrier;
class UAGX_RigidBodyComponent;
class UAGX_SphereShapeComponent;
class UAGX_BoxShapeComponent;
class UAGX_CylinderShapeComponent;
class UAGX_CapsuleShapeComponent;
class UAGX_TrimeshShapeComponent;
class UAGX_ShapeMaterial;
class UAGX_ContactMaterialAsset;
class UAGX_HingeConstraintComponent;
class UAGX_PrismaticConstraintComponent;
class UAGX_BallConstraintComponent;
class UAGX_CylindricalConstraintComponent;
class UAGX_DistanceConstraintComponent;
class UAGX_LockConstraintComponent;
class UAGX_TwoBodyTireComponent;
class UAGX_CollisionGroupDisablerComponent;
class UAGX_ContactMaterialRegistrarComponent;
class UAGX_WireComponent;

// Unreal Engine classes.
class AActor;
class USceneComponent;
class UMaterialInstanceConstant;
class UStaticMesh;

/**
 * An Unreal Engine side helper that creates `[UA]AGX_.*` objects from Barrier objects read from an
 * AGX Dynamics archive or URDF file.
 */
struct FAGX_SimObjectsImporterHelper
{
public:
	/** Create a new UAGX_RigidBodyComponent in the given actor. */
	UAGX_RigidBodyComponent* InstantiateBody(const FRigidBodyBarrier& Barrier, AActor& Owner);

	void UpdateComponent(const FRigidBodyBarrier& Barrier, UAGX_RigidBodyComponent& Component);

	UAGX_SphereShapeComponent* InstantiateSphere(
		const FSphereShapeBarrier& Sphere, AActor& Owner, const FRigidBodyBarrier* Body = nullptr);

	UAGX_BoxShapeComponent* InstantiateBox(
		const FBoxShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* Body = nullptr);

	UAGX_CylinderShapeComponent* InstantiateCylinder(
		const FCylinderShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	UAGX_CapsuleShapeComponent* InstantiateCapsule(
		const FCapsuleShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	UAGX_TrimeshShapeComponent* InstantiateTrimesh(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	UStaticMeshComponent* InstantiateRenderData(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	void UpdateAsset(
		const FShapeMaterialBarrier& Barrier, UAGX_ShapeMaterial& Asset);

	UAGX_ShapeMaterial* InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier);

	UAGX_ContactMaterialAsset* InstantiateContactMaterial(
		const FContactMaterialBarrier& Barrier, AActor& Owner);

	UAGX_HingeConstraintComponent* InstantiateHinge(const FHingeBarrier& Barrier, AActor& Owner);

	UAGX_PrismaticConstraintComponent* InstantiatePrismatic(
		const FPrismaticBarrier& Barrier, AActor& Owner);

	UAGX_BallConstraintComponent* InstantiateBallConstraint(
		const FBallJointBarrier& Barrier, AActor& Owner);

	UAGX_CylindricalConstraintComponent* InstantiateCylindricalConstraint(
		const FCylindricalJointBarrier& Barrier, AActor& Owner);

	UAGX_DistanceConstraintComponent* InstantiateDistanceConstraint(
		const FDistanceJointBarrier& Barrier, AActor& Owner);

	UAGX_LockConstraintComponent* InstantiateLockConstraint(
		const FLockJointBarrier& Barrier, AActor& Owner);

	UAGX_TwoBodyTireComponent* InstantiateTwoBodyTire(
		const FTwoBodyTireBarrier& Barrier, AActor& Owner, bool IsBlueprintOwner = false);

	UAGX_CollisionGroupDisablerComponent* InstantiateCollisionGroupDisabler(
		AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs);

	UAGX_WireComponent* InstantiateWire(const FWireBarrier& Barrier, AActor& Owner);

	void UpdateComponent(UAGX_ReImportComponent& Component);

	UAGX_ReImportComponent* InstantiateReImportComponent(AActor& Owner);

	/**
	 * We currently do not have full Observer Frame support in AGX Dynamics for Unreal, i.e. there
	 * is no Observer Frame Component or Barrier. The coordinate frame defined by an Observer Frame
	 * can still be useful, for example to act as attachment points for constraints, so for now we
	 * instantiate a plain Scene Component for each imported Observer Frame.
	 *
	 * Replace all parameters with a FObserverFrameBarrier once we implement full Observer Frame
	 * support.
	 *
	 * \param Name The name of the Observer Frame.
	 * \param BodyGuid The GUID of the Rigid Body that the Observer Frame is attached to.
	 * \param Transform The transformation of the Observer Frame relative to the Rigid Body.
	 * \param Owner The Actor in which the new Scene Component is to be created.
	 * \return The Scene Component created at the location of the Observer Frame.
	 */
	USceneComponent* InstantiateObserverFrame(
		const FString& Name, const FGuid& BodyGuid, const FTransform& Transform, AActor& Owner);

	UAGX_RigidBodyComponent* GetBody(
		const FRigidBodyBarrier& Barrier, bool LogErrorIfNotFound = true);

	using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;
	FBodyPair GetBodies(const FConstraintBarrier& Barrier);

	UAGX_ShapeMaterial* GetShapeMaterial(const FShapeMaterialBarrier& Barrier);

	using FShapeMaterialPair = std::pair<UAGX_ShapeMaterial*, UAGX_ShapeMaterial*>;
	FShapeMaterialPair GetShapeMaterials(const FContactMaterialBarrier& ContactMaterial);

	/*
	 * Must be called at the end of an import.
	 */
	void FinalizeImport();

	explicit FAGX_SimObjectsImporterHelper(const FAGX_ImportSettings& InImportSettings);
	explicit FAGX_SimObjectsImporterHelper(
		const FAGX_ImportSettings& InImportSettings, const FString& InDirectoryName);

	const FAGX_ImportSettings ImportSettings;
	const FString SourceFileName;
	const FString DirectoryName;

private:
	TMap<FGuid, FAssetToDiskInfo> RestoredMeshes;
	TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
	TMap<FGuid, UAGX_ShapeMaterial*> RestoredShapeMaterials;
	TMap<FGuid, UMaterialInstanceConstant*> RestoredRenderMaterials;
};
