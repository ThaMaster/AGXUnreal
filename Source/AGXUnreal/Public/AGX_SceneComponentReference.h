#pragma once

#include "AGX_SceneComponentReference.generated.h"

class AActor;

/**
 * A reference to a USceneComponent.
 *
 * The intention is that it should be used much like Actors pointers can, but limitations in the
 * Unreal Editor forces us to do some tricks and workarounds. There is no Component picker, so the
 * user must first pick an Actor that owns the Component and then select the wanted component from a
 * combo box of names. There is no actual pointer to the Componet stored in the
 * FAGX_SceneComponentReference, on the the name, so renaming the Component will break the
 * reference. This is a serious limitaiton. Also, while building the a Blueprint Actor in the
 * Blueprint editor there is no actual Actor yet, so the Actor picker cannot be used to select the
 * Actor that will be created when the Bluepritn is instantiated. The SceneComponentReference
 * provdies a getter with an InOwningActor parameter for this purpose.
 *
 * The SceneComponentReference supports caching of the SceneComponent through the
 * CacheCurrentSceneComponent member function. Only call this once the RigidBodyReference has been
 * fully fomred, i.e., the OwningActor property set to the final Actor and when the referenced
 * SceneComponent has been given its final name.
 *
 * \todo The implementation is very similar to UAGX_RigidBodyComponent. Find what can be shared and
 * put somewhere.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_SceneComponentReference
{
	GENERATED_BODY()

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
