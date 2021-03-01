#include "AGX_RigidBodyReference.h"

#include "AGX_RigidBodyComponent.h"

#include "GameFramework/Actor.h"

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
	if (Cache != nullptr)
	{
		return Cache;
	}
#if AGXUNREAL_RIGID_BODY_REFERENCE_REFACTOR
	else if (IsValid(OwningActor))
	{
		return FindBody(OwningActor, BodyName, bSearchChildActors);
	}
#else
	else if (OwningActor.IsValid())
	{
		return FindBody(OwningActor.Get(), BodyName, bSearchChildActors);
	}
	else if (FallbackOwningActor != nullptr)
	{
		return FindBody(FallbackOwningActor, BodyName, bSearchChildActors);
	}
#endif
	else
	{
		return nullptr;
	}
}

AActor* FAGX_RigidBodyReference::GetOwningActor() const
{
#if AGXUNREAL_RIGID_BODY_REFERENCE_REFACTOR
	return OwningActor;
#else
	return OwningActor.Get();
#endif
}

void FAGX_RigidBodyReference::CacheCurrentRigidBody()
{
	InvalidateCache();
#if AGXUNREAL_RIGID_BODY_REFERENCE_REFACTOR
	if (!IsValid(OwningActor))
#else
	if (!OwningActor.IsValid())
#endif
	{
		return;
	}
#if AGXUNREAL_RIGID_BODY_REFERENCE_REFACTOR
	Cache = FindBody(OwningActor, BodyName, bSearchChildActors);
#else
	Cache = FindBody(OwningActor.Get(), BodyName, bSearchChildActors);
#endif
}

void FAGX_RigidBodyReference::InvalidateCache()
{
	Cache = nullptr;
}
