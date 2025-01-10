// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UAGX_MergeSplitThresholdsBase;
class UAGX_ModelSourceComponent;
class UAGX_RigidBodyComponent;
class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;

/*
 * Todo: add comment.
 */
struct AGXUNREAL_API FAGX_AGXToUeContext
{
	TUniquePtr<TMap<FGuid, UAGX_RigidBodyComponent*>> RigidBodies;
	TUniquePtr<TMap<FGuid, UAGX_ShapeComponent*>> Shapes;

	TUniquePtr<TMap<FGuid, UAGX_MergeSplitThresholdsBase*>> MSThresholds;
	TUniquePtr<TMap<FGuid, UAGX_ShapeMaterial*>> ShapeMaterials;

	UAGX_ModelSourceComponent* ModelSourceComponent {nullptr};
};
