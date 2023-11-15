// Copyright 2023, Algoryx Simulation AB.

#include "AGX_RigidBodyReference.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

FAGX_RigidBodyReference::FAGX_RigidBodyReference()
	: FAGX_ComponentReference(UAGX_RigidBodyComponent::StaticClass())
{
}

UAGX_RigidBodyComponent* FAGX_RigidBodyReference::GetBodyComponent() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}

UAGX_RigidBodyComponent* FAGX_RigidBodyReference::GetRigidBodyComponent() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}

UAGX_RigidBodyComponent* FAGX_RigidBodyReference::GetRigidBody() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}
