// Copyright 2023, Algoryx Simulation AB.

#include "AGX_BodyReference.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

FAGX_BodyReference::FAGX_BodyReference()
	: FAGX_ComponentReference(UAGX_RigidBodyComponent::StaticClass())
{
}

UAGX_RigidBodyComponent* FAGX_BodyReference::GetBodyComponent() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}

UAGX_RigidBodyComponent* FAGX_BodyReference::GetRigidBodyComponent() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}

UAGX_RigidBodyComponent* FAGX_BodyReference::GetRigidBody() const
{
	return Super::GetComponent<UAGX_RigidBodyComponent>();
}
