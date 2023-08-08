// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "AGX_ComponentReference.generated.h"

class UActor;
class UActorComponent;


USTRUCT()
struct AGXUNREAL_API FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_ComponentReference();
	FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType);

	UPROPERTY(
		EditInstanceOnly, Category = "Component Reference",
		Meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor {nullptr};

	UPROPERTY(
		EditAnywhere, Category = "Component Reference",
		Meta = (Tooltip = "The name of the Component."))
	FName Name {NAME_None};

	UPROPERTY(EditAnywhere, Category = "Component Reference")
	uint8 bSearchChildActors : 1;

	UActorComponent* GetComponent() const;

	void GetCompatibleComponents(TArray<UActorComponent*>& OutComponents) const;

	template<typename T>
	T* GetComponent() const;

	AActor* GetOwningActor() const;

	// @todo Should we add caching? Is it safe, even in the face of Blueprint Reconstruction? Can
	// we find cases where it is guaranteed to be safe?

	TSubclassOf<UActorComponent> ComponentType;
};

template<typename T>
T* FAGX_ComponentReference::GetComponent() const
{
	return Cast<T>(GetComponent());
}
