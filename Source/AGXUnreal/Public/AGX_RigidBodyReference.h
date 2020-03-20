#pragma once

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

/**
 * A reference to a UAGX_RigidBodyComponent.
 *
 * The intention is that it should be used much like Actor pointers can, but limitations in the
 * Unreal Editor forces us to do some tricks and workarounds. There is no Component picker, so the
 * user must first pick an Actor that owns the Component and then select the wanted component from a
 * combo box of body names. There is no actual pointer to the Component stored in the
 * FAGX_RigidBodyReference, only the name, so renaming the body will break the reference. This is a
 * serious limitation. Also, while building a Blueprint Actor in the Blueprint editor there is no
 * actual Actor yet, so the Actor picker cannot be used to select the Actor that will be created
 * when the Blueprint is instantiated. The RigidBodyReference provides a getter with an
 * InOwningActor parameter for this purpose.
 *
 * The RigidBodyReference supports caching of the RigidBodyComponent through the CacheRigidBody
 * member function. Only call this once the RigidBodyReference has been fully formed, i.e., the
 * OwningActor property set to the final Actor and when the references RigidBodyComponent has been
 * given its final name.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference
{
	GENERATED_BODY()

	// Using a SoftObjectPtr instead of a raw pointer because we use FAGX_RigidBodyReference in the
	// AGXUnreal Mode for constraint creation and Unreal Engine does not allow raw pointers to world
	// objects being held by non-world objects. The constraint creation Mode is a non-world object.
	// When used in a constraint that exists in the level the OwningActor should always point to an
	// Actor in the same world as the constraint.
	//
	// A LazyObjectPtr could be used as well, I think. I don't know which of the two would be better
	// in this case.
	//
	// The EditInstanceOnly specifier is passed because Actor Blueprints should be self-contained
	// so all Actor references should implicitly point to the Actor that will be created when the
	// Blueprint is instantiated in a level. I can imagine cases where the user want to create a
	// Blueprint that is bound to a particular level and uses references to objects within the
	// level, but that will not be supported at this stage.
	UPROPERTY(
		EditInstanceOnly, Category = "AGX Dynamics",
		meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	TSoftObjectPtr<AActor> OwningActor;

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	FName BodyName;

	/// \todo It may be possible to do this with a UAGX_RigidBodyComponent
	/// property instead of the name. The idea is to have a PropertyChanged
	/// callback that populate a combobox with all the UAGX_RigidBodyComponents
	/// in the Actor whenever the Actor property is changed.

	/// If true, then search not only within OwningActor's Components, but also recursively through
	/// ChildActorComponents as well.
	UPROPERTY(EditInstanceOnly, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	// A fallback owning Actor that can be used when we want to specify an Actor to search for
	// RigidBodies in but don't want to expose it to the user and don't want to accidentally leak
	// the Actor pointer into an actual game session. Used, for example, while building Blueprint
	// Actors in the Blueprint editor during which the FallbackOwningActor points to the Blueprint
	// itself.
	//
	// FallbackOwningActor is ignored while OwningActor is not nullptr.
	// Should be cleared on BeginPlay in the object holds this RigidBodyReference.
	// Intentionally not a UPROPERTY.
	AActor* FallbackOwningActor;

	/**
	 * Returns the cached body if there is one. Otherwise, performs a search to find and return the
	 * RigidBody that this reference currently references. Can return nullptr. Will return nullptr
	 * if both OwningActor and FallbackOwningActor is nullptr and if neither of them, and all of
	 * their child actors if bSearchChildActors is true, contain a UAGX_RigidBodyComponent named
	 * BodyName.
	 *
	 * FallbackOwningActor will not be searched if OwningActor is non-nullptr even if no matching
	 * RigidBody is found in OwningActor.
	 *
	 * @return The UAGX_RigidBodyComponent that this FAGX_RigidBodyReference currently references.
	 */
	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Get the Actor that owns the RigidBody that this reference currently references. Can return
	 * nullptr. Can return non-null even if no RigidBody is currently the target of this reference.
	 * In that case the returned Actor is the Actor that will be searched the next time GetRigidBody
	 * is called.
	 */
	AActor* GetOwningActor() const;

	/**
	 * Forget the currently cached body, if any, and perform a new search through
	 * OwningActor's components. The found RigidBody will be cached and future calls to GetRigidBody
	 * will return the cached body even if the body is renamed or removed, or if OwningActor or
	 * BodyName is changed, until either a new call to CacheCurrentRigidBody or InvalidateCache is
	 * called.
	 *
	 * Will not search in FallbackOwningActor, only OwningActor.
	 *
	 * \todo The non-caching of FallbackOwningActor may make the Unreal Editor sluggish. We should
	 * convince ourselves that we can keep the cache correct.
	 */
	void CacheCurrentRigidBody();

	/**
	 * Forget the currently cached body. Should be called when any of the search
	 * parameters is changed and when the referenced body is renamed.
	 */
	void InvalidateCache();

private:
	UAGX_RigidBodyComponent* Cache;
};
