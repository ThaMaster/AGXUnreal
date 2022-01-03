// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "AGX_SceneComponentReference.generated.h"

class AActor;

/**
 * A reference to a USceneComponent.
 *
 * The intention is that it should be used much like Actors pointers can, to a SceneComponent
 * instead of an Actor, but limitations in the Unreal Editor forces us to do some tricks and
 * workarounds. There is no built-in Component picker, so the user must first pick an Actor that
 * owns the Component and then select the wanted component by entering its name in a text field.
 * Populating a combo box with the available names is a possible future extension. Except for the
 * cache, there is no actual pointer to the Component stored in the FAGX_SceneComponentReference,
 * only the name, so renaming the Component will break the reference. This is a serious limitation.
 * Also, while building the a Blueprint Actor in the Blueprint editor there is no actual Actor yet,
 * so the Actor picker cannot be used to select the Actor that will be created when the Blueprint is
 * instantiated.  For this reason all Components that include a SceneComponentReference should set
 * OwningActor to GetTypedOuter<AActor>() in PostInitProperties.
 *
 * void UMyComponent::PostInitProperties()
 * {
 *  	Super::PostInitProperties();
 *  	MySceneComponentReference.OwningActor = GetTypedOuter<AActor>();
 * }
 *
 * This establishes the so-called local scope for the reference. Unless another OwningActor is
 * specified, the reference will search within the Actor that the Component is contained within. The
 * OwningActor set in PostInitProperties will we overwritten by deserialization if the object is
 * created from something else, such as part of a Play-in-Editor session or loaded from disk as part
 * of a cooked build.
 *
 * The SceneComponentReference supports caching of the SceneComponent through the
 * CacheCurrentSceneComponent member function. Only call this once the SceneComponentReference has
 * been fully formed, i.e., the OwningActor property set to the final Actor and the referenced
 * SceneComponent been given its final name. BeginPlay is often a good choice.
 *
 * \todo The implementation is very similar to FAGX_SceneComponentReference. Find what can be shared
 * and put somewhere.
 */
USTRUCT() struct AGXUNREAL_API FAGX_SceneComponentReference
{
	GENERATED_BODY()

	UPROPERTY(
		EditInstanceOnly, Category = "AGX Dynamics",
		meta = (Tooltip = "The Actor that owns the SceneComponent"))
	AActor* OwningActor = nullptr;

	/**
	 * The name of the SceneComponent that we should find within OwningActor. Setting this to
	 * NAME_None means that the Actor's RootComponent is referenced.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		Meta = (Tooltip = "The name of the SceneComponent."))
	FName SceneComponentName;

	/**
	 * If true, then search not only within OwningActor's Components, but also recursively through
	 * ChildActorComponents as well.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	uint8 bSearchChildActors : 1;

	USceneComponent* GetSceneComponent() const;

	AActor* GetOwningActor() const;

	void Set(AActor* InOwningActor, FName InSceneComponentName);

	/// Make this reference point to nothing.
	void Clear();

	void CacheCurrentSceneComponent();

	void InvalidateCache();

private:
	USceneComponent* Cache;
};
