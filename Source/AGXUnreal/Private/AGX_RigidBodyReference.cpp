// Copyright 2023, Algoryx Simulation AB.

#include "AGX_RigidBodyReference.h"

#include "AGX_RigidBodyComponent.h"

#include "GameFramework/Actor.h"

FAGX_RigidBodyReference::FAGX_RigidBodyReference()
	: bSearchChildActors(false)
{
}

namespace
{
	UAGX_RigidBodyComponent* FindBody(
		AActor* OwningActor, const FName& BodyName, bool bSearchChildActors)
	{
		TArray<UAGX_RigidBodyComponent*> Bodies;
		OwningActor->GetComponents(Bodies, bSearchChildActors);
		UAGX_RigidBodyComponent** It = Bodies.FindByPredicate(
			[BodyName](UAGX_RigidBodyComponent* Body) { return BodyName == Body->GetFName(); });
		if (It == nullptr)
		{
			return nullptr;
		}
		return *It;
	}
}

UAGX_RigidBodyComponent* FAGX_RigidBodyReference::GetRigidBody() const
{
	// If the Rigid Body that we reference is in a Blueprint, i.e. a Template Component, this will
	// currently always return nullptr. It would be nice to be able to detect this case, and find
	// the Rigid Body Template Component by searching the Blueprint SCS Node tree and returning it.
	// Since this is only a Struct, we are not able to get the Blueprint from here, if we are in
	// one. A bit of re-design of this Struct may be needed to get this to work properly.

	if (Cache != nullptr)
	{
		return Cache;
	}
	else if (IsValid(OwningActor))
	{
		return FindBody(OwningActor, BodyName, bSearchChildActors);
	}
	else
	{
		return nullptr;
	}
}

FRigidBodyBarrier* FAGX_RigidBodyReference::GetRigidBodyBarrier() const
{
	UAGX_RigidBodyComponent* Component = GetRigidBody();
	if (Component == nullptr)
	{
		return nullptr;
	}
	return Component->GetNative();
}

AActor* FAGX_RigidBodyReference::GetOwningActor() const
{
	return OwningActor;
}

void FAGX_RigidBodyReference::CacheCurrentRigidBody()
{
	InvalidateCache();
	if (!IsValid(OwningActor))
	{
		return;
	}
	Cache = FindBody(OwningActor, BodyName, bSearchChildActors);
}

void FAGX_RigidBodyReference::InvalidateCache()
{
	Cache = nullptr;
}
