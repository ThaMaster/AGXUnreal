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
 * FAGX_RigidBodyReference, only the name, so renaiming the body will break the reference. This is a
 * serious limitaiton. Also, while building a Blueprint Actor in the Blueprint editor there is no
 * actual Actor yet, so the Actor picker cannot be used to select the Actor that will be created
 * when the Blueprint is instantiated. Instead the user must leave the OwningActor field blank and
 * free-type the body name in a text box.
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
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
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
	UPROPERTY(EditAnywhere, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	/**
	 * Get the RigidBody that this reference currently references. Can return nullptr. Will return
	 * nullptr if OwningActor is nullptr and, otherwise, if OwningActor, and none of it's child
	 * actors if bSearchChildActors is true, doesn't containt a UAGX_RigidBodyComponent named
	 * BodyName.
	 *
	 * The returned RigidBody will be cached and future calls to GetRigidBody will return the same
	 * body even if the body is renamed or OwningActor or BodyName is changed until either
	 * UpdateCache or InvalidateCache is called.
	 */
	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Get the Actor that owns the RigidBody that this reference currently references. Can return
	 * nullptr. Can return non-null even if no RigidBody is currently the target of this reference.
	 * In that case the returned Actor is the Actor that will be seached the next time GetRigidBody
	 * is called.
	 */
	AActor* GetOwningActor() const;

	/**
	 * Forget the currently cached body and perform a new search through
	 * OwningActor's components.
	 */
	void UpdateCache() const;

	/**
	 * Forget the currently cached body. Should be called when any of the search
	 * parameters is changed and when the referenced body is renamed.
	 */
	void InvalidateCache() const;

private:
	mutable UAGX_RigidBodyComponent* Cache;
};
