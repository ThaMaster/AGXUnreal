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
	else if (IsValid(OwningActor))
	{
		return FindBody(OwningActor, BodyName, bSearchChildActors);
	}
	else
	{
		return nullptr;
	}
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
