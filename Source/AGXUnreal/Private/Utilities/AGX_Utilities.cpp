// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_Utilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGXUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

void UAGX_AGXUtilities::AddParentVelocity(
	UAGX_RigidBodyComponent* Parent, UAGX_RigidBodyComponent* Body)
{
	if (Parent == nullptr || !Parent->HasNative() || Body == nullptr || !Body->HasNative())
		return;

	FAGXUtilities::AddParentVelocity(*Parent->GetNative(), *Body->GetNative());
}

void UAGX_AGXUtilities::AddParentVelocityMany(
	UAGX_RigidBodyComponent* Parent, const TArray<UAGX_RigidBodyComponent*>& Bodies)
{
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		if (Body == Parent)
			continue;

		AddParentVelocity(Parent, Body);
	}
}
