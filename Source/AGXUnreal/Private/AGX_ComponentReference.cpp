// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ComponentReference.h"

FAGX_ComponentReference::FAGX_ComponentReference()
	: FAGX_ComponentReference(TSubclassOf<UActorComponent>())
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
		TSubclassOf<UActorComponent> ComponentType, AActor* OwningActor, bool bSearchChildActors)
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
		TSubclassOf<UActorComponent> ComponentType, AActor* OwningActor, const FName& Name,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components =
			GetCompatibleComponents(ComponentType, OwningActor, bSearchChildActors);
		UActorComponent** It = Components.FindByPredicate(
			[&Name](UActorComponent* Component) { return Component->GetFName() == Name; });
		return It != nullptr ? *It : nullptr;
	}
}

UActorComponent* FAGX_ComponentReference::GetComponent() const
{
	return IsValid(OwningActor) ? FAGX_ComponentReference_helpers::FindComponent(
									  ComponentType, OwningActor, Name, bSearchChildActors)
								: nullptr;
}

TArray<UActorComponent*> FAGX_ComponentReference::GetCompatibleComponents() const
{
	return FAGX_ComponentReference_helpers::GetCompatibleComponents(
		ComponentType, OwningActor, bSearchChildActors);
}

UActorComponent* UAGX_ComponentReference_FL::GetComponent(FAGX_ComponentReference& Reference)
{
	return Reference.GetComponent();
}
