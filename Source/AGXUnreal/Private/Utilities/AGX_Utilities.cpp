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

FVector UAGX_AGXUtilities::CalculateCenterOfMass(const TArray<UAGX_RigidBodyComponent*>& Bodies)
{
	FVector Com = FVector::ZeroVector;
	float TotalMass = 0.f;
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		if (Body == nullptr || Body->GetWorld() == nullptr || !Body->GetWorld()->IsGameWorld())
			continue;

		const auto Mass = Body->GetMass();
		const auto Cm = Body->GetCenterOfMassPosition();
		Com += Cm * Mass;
		TotalMass += Mass;
	}

	if (!FMath::IsNearlyZero(TotalMass))
		Com /= TotalMass;

	return Com;
}
