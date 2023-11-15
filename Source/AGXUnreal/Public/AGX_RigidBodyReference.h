// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

/**
 * A reference to an UAGX_RigidBodyComponent.
 *
 * Replaces the custom / type-specific FAGX_RigidBodyComponent with this FAGX_ComponentReference
 * based implementation.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_RigidBodyReference();

	// TODO Pick a name for the body getter.

	UAGX_RigidBodyComponent* GetBodyComponent() const;
	UAGX_RigidBodyComponent* GetRigidBodyComponent() const;
	UAGX_RigidBodyComponent* GetRigidBody() const;
};
