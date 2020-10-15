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
 * Populating a combo box with the available names is a possible future extension. There is no
 * actual pointer to the Component stored in the FAGX_SceneComponentReference, only the name, so
 * renaming the Component will break the reference. This is a serious limitation. Also, while
 * building the a Blueprint Actor in the Blueprint editor there is no actual Actor yet, so the Actor
 * picker cannot be used to select the Actor that will be created when the Blueprint is
 * instantiated. The SceneComponentReference provides a FallbackOwningActor for this purpose. The
 * FallbackOwningActor should be cleared and the OwningActor set to something sensible in the
 * PostLoad callback of the Component containing the USceneComponentReference.
 *
 * The SceneComponentReference supports caching of the SceneComponent through the
 * CacheCurrentSceneComponent member function. Only call this once the RigidBodyReference has been
 * fully formed, i.e., the OwningActor property set to the final Actor and the referenced
 * SceneComponent been given its final name.
 *
 * \todo The implementation is very similar to FAGX_RigidBodyReference. Find what can be shared and
 * put somewhere.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_SceneComponentReference
{
	GENERATED_BODY()

	// Don't like using soft references here. See comment in FAGX_RigidBodyReference.
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		meta = (Tooltip = "The Actor that owns the SceneComponent"))
	TSoftObjectPtr<AActor> OwningActor;

	/// The name of the SceneComponent that we should find within OwningActor. Setting this to
	/// NAME_None means that the Actor's RootComponent is referenced.
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	FName SceneComponentName;

	UPROPERTY(EditAnywhere, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	AActor* FallbackOwningActor;

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
