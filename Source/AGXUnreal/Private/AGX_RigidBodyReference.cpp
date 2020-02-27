#include "AGX_RigidBodyReference.h"

#include "AGX_RigidBodyComponent.h"

#include "GameFramework/Actor.h"

UAGX_RigidBodyComponent* FAGX_RigidBodyReference::GetRigidBody()
{
	if (Cache == nullptr)
	{
		UpdateCache();
	}

	return Cache;
}

namespace
{
	UAGX_RigidBodyComponent* FindBody(AActor* OwningActor, const FName& BodyName, bool bSearchChildActors)
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

void FAGX_RigidBodyReference::UpdateCache()
{
	InvalidateCache();
	if (OwningActor == nullptr)
	{
		return;
	}
	Cache = FindBody(OwningActor, BodyName, bSearchChildActors);
}

void FAGX_RigidBodyReference::InvalidateCache()
{
	Cache = nullptr;
}
