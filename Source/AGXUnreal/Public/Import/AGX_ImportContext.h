// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImporterSettings;

class UAGX_CollisionGroupDisablerComponent;
class UAGX_ConstraintComponent;
class UAGX_ContactMaterial;
class UAGX_ContactMaterialRegistrarComponent;
class UAGX_MergeSplitThresholdsBase;
class UAGX_ModelSourceComponent;
class UAGX_ObserverFrameComponent;
class UAGX_RigidBodyComponent;
class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;
class UMaterialInstanceConstant;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/*
 * Todo: add comment.
 */
struct AGXUNREAL_API FAGX_ImportContext
{
	TUniquePtr<TMap<FGuid, UAGX_RigidBodyComponent*>> RigidBodies;
	TUniquePtr<TMap<FGuid, UAGX_ShapeComponent*>> Shapes;
	TUniquePtr<TMap<FGuid, UAGX_ConstraintComponent*>> Constraints;
	TUniquePtr<TMap<FGuid, UAGX_ObserverFrameComponent*>> ObserverFrames;

	// The key is the GUID of the Shape Component for which the render data Static Mesh
	// Component has been created.
	TUniquePtr<TMap<FGuid, UStaticMeshComponent*>> RenderStaticMeshCom;

	TUniquePtr<TMap<FGuid, UStaticMeshComponent*>> CollisionStaticMeshCom;

	TUniquePtr<TMap<FGuid, UMaterialInterface*>> RenderMaterials;

	// For render meshes, the GUID is taken from the RenderData.
	TUniquePtr<TMap<FGuid, UStaticMesh*>> RenderStaticMeshes;

	TUniquePtr<TMap<FGuid, UStaticMesh*>> CollisionStaticMeshes;

	TUniquePtr<TMap<FGuid, UAGX_MergeSplitThresholdsBase*>> MSThresholds;
	TUniquePtr<TMap<FGuid, UAGX_ShapeMaterial*>> ShapeMaterials;
	TUniquePtr<TMap<FGuid, UAGX_ContactMaterial*>> ContactMaterials;

	UAGX_ModelSourceComponent* ModelSourceComponent {nullptr};
	UAGX_ContactMaterialRegistrarComponent* ContactMaterialRegistrar{nullptr};
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler {nullptr};

	FGuid SessionGuid;

	const FAGX_ImporterSettings* Settings {nullptr};
};
