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
	UActorComponent* FindComponent(
		TSubclassOf<UActorComponent> Type, AActor* OwningActor, const FName& Name,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components;
		OwningActor->GetComponents(Type, Components, bSearchChildActors);
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

void FAGX_ComponentReference::GetCompatibleComponents(TArray<UActorComponent*>& OutComponents) const
{
	OutComponents.Empty();
	if (OwningActor == nullptr)
	{
		return;
	}
	OwningActor->GetComponents(ComponentType, OutComponents, bSearchChildActors);
}

AActor* FAGX_ComponentReference::GetOwningActor() const
{
	return OwningActor;
}
