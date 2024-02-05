// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_SceneComponentReference.generated.h"

class AActor;

/**
 * A reference to a USceneComponent.
 *
 * See comment on FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_SceneComponentReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_SceneComponentReference();

	USceneComponent* GetSceneComponent() const;
};
