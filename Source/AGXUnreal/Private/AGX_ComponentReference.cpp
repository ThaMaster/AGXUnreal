// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ComponentReference.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

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

FAGX_ComponentReference& FAGX_ComponentReference::operator=(const FAGX_ComponentReference& Other)
{
	OwningActor = Other.OwningActor;
	// Intentionally not copying Local Scope.
	Name = Other.Name;
	bSearchChildActors = Other.bSearchChildActors;
	return *this;
}

AActor* FAGX_ComponentReference::GetScope() const
{
	return IsValid(OwningActor) ? OwningActor : LocalScope;
}

namespace FAGX_ComponentReference_helpers
{
	TArray<UActorComponent*> GetCompatibleComponents(
		TSubclassOf<UActorComponent> ComponentType, const AActor* const Scope,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components;
		if (Scope == nullptr)
		{
			return Components;
		}
		Scope->GetComponents(ComponentType, Components, bSearchChildActors);
		return Components;
	}

	UActorComponent* FindComponent(
		TSubclassOf<UActorComponent> ComponentType, const AActor* const Scope, const FName& Name,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components =
			GetCompatibleComponents(ComponentType, Scope, bSearchChildActors);
		UActorComponent** It = Components.FindByPredicate(
			[&Name](UActorComponent* Component) { return Component->GetFName() == Name; });
		return It != nullptr ? *It : nullptr;
	}
}

UActorComponent* FAGX_ComponentReference::GetComponent() const
{
	const AActor* const Scope = IsValid(OwningActor) ? OwningActor : LocalScope;
	return FAGX_ComponentReference_helpers::FindComponent(
		ComponentType, Scope, Name, bSearchChildActors);
}

TArray<UActorComponent*> FAGX_ComponentReference::GetCompatibleComponents() const
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("ComponentReference %p getting compatible components from Owning Actor %p, Local "
			 "Scope %p."),
		this, OwningActor, LocalScope);
	const AActor* const Scope = IsValid(OwningActor) ? OwningActor : LocalScope;
	return FAGX_ComponentReference_helpers::GetCompatibleComponents(
		ComponentType, Scope, bSearchChildActors);
}

UActorComponent* UAGX_ComponentReference_FL::GetComponent(FAGX_ComponentReference& Reference)
{
	return Reference.GetComponent();
}
