// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"

FAGX_ComponentReference::FAGX_ComponentReference()
	: FAGX_ComponentReference(UActorComponent::StaticClass())
{
}

FAGX_ComponentReference::FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType)
	: bSearchChildActors(false)
	, ComponentType(InComponentType)
{
}

namespace FAGX_ComponentReference_helpers
{
	TArray<UActorComponent*> GetCompatibleComponents(
		TSubclassOf<UActorComponent> ComponentType, const AActor* OwningActor, bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components;
		if (OwningActor == nullptr)
		{
			return Components;
		}
		OwningActor->GetComponents(ComponentType, Components, bSearchChildActors);
		return Components;
	}

	UActorComponent* FindComponent(
		TSubclassOf<UActorComponent> ComponentType, const AActor* OwningActor, const FName& Name,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components =
			GetCompatibleComponents(ComponentType, OwningActor, bSearchChildActors);
		UActorComponent** It = Components.FindByPredicate(
			[&Name](UActorComponent* Component) { return Component->GetFName() == Name; });
		return It != nullptr ? *It : nullptr;
	}
}

UActorComponent* FAGX_ComponentReference::GetComponent(const AActor* LocalScope) const
{
	const AActor* Scope = IsValid(OwningActor) ? OwningActor : LocalScope;
	if (!IsValid(Scope))
	{
		return nullptr;
	}
	return FAGX_ComponentReference_helpers::FindComponent(
		ComponentType, Scope, Name, bSearchChildActors);
}

TArray<UActorComponent*> FAGX_ComponentReference::GetCompatibleComponents(const AActor* LocalScope) const
{
	return FAGX_ComponentReference_helpers::GetCompatibleComponents(
		ComponentType, LocalScope, bSearchChildActors);
}

UActorComponent* UAGX_ComponentReference_FL::GetComponent(
	FAGX_ComponentReference& Reference, const AActor* LocalScope)
{
	return Reference.GetComponent(LocalScope);
}
