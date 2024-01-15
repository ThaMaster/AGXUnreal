// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

/**
 * A reference to an UAGX_RigidBodyComponent.
 *
 * See comment in FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_RigidBodyReference();

	UAGX_RigidBodyComponent* GetRigidBody() const;
};
