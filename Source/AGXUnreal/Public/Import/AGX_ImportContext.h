// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImporterSettings;

class UAGX_MergeSplitThresholdsBase;
class UAGX_ModelSourceComponent;
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

	UAGX_ModelSourceComponent* ModelSourceComponent {nullptr};

	FGuid SessionGuid;

	const FAGX_ImporterSettings* Settings {nullptr};
};
