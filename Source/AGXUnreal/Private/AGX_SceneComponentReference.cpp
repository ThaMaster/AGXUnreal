#include "AGX_SceneComponentReference.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	USceneComponent* FindComponent(
		AActor& OwningActor, const FName& ComponentName, bool bSearchChildActors)
	{
		if (ComponentName == NAME_None)
		{
			return OwningActor.GetRootComponent();
		}
		else
		{
			TArray<USceneComponent*> Components;
			OwningActor.GetComponents(Components, bSearchChildActors);
			USceneComponent** It =
				Components.FindByPredicate([ComponentName](USceneComponent* SceneComponent) {
					return ComponentName == SceneComponent->GetFName();
				});
			if (It == nullptr)
			{
				return nullptr;
			}
			return *It;
		}
	}
}

USceneComponent* FAGX_SceneComponentReference::GetSceneComponent() const
{
	if (Cache != nullptr)
	{
		return Cache;
	}
	else if (OwningActor.IsValid())
	{
		return FindComponent(*OwningActor.Get(), SceneComponentName, bSearchChildActors);
	}
	else if (FallbackOwningActor != nullptr)
	{
		return FindComponent(*FallbackOwningActor, SceneComponentName, bSearchChildActors);
	}
	else
	{
		return nullptr;
	}
}

AActor* FAGX_SceneComponentReference::GetOwningActor() const
{
	return OwningActor.Get();
}

void FAGX_SceneComponentReference::Set(AActor* InOwningActor, FName InSceneComponentName)
{
	OwningActor = InOwningActor;
	SceneComponentName = InSceneComponentName;
}

void FAGX_SceneComponentReference::Clear()
{
	OwningActor = nullptr;
	SceneComponentName = NAME_None;
}

void FAGX_SceneComponentReference::CacheCurrentSceneComponent()
{
	InvalidateCache();
	if (!OwningActor.IsValid())
	{
		return;
	}
	Cache = FindComponent(*OwningActor.Get(), SceneComponentName, bSearchChildActors);
}

void FAGX_SceneComponentReference::InvalidateCache()
{
	Cache = nullptr;
}
