#pragma once

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference
{
	GENERATED_BODY()

	// Using a SoftObjectPtr instead of a raw pointer because we use FAGX_RigidBodyReference in the
	// AGXUnreal Mode for constraint creation and Unreal Engine does not allow raw pointers to world
	// objects being held by non-world objects. The constraint creation Mode is a non-world object.
	// When used to define a constraint the OwningActor should always point to an Actor in the same
	// world as the constraint.
	//
	// A LazyObjectPtr could be used as well, I think. I don't understand which of the two would be
	// better in this case.
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

	UPROPERTY(EditAnywhere, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	/**
	 * Get the RigidBody that this reference currently references. Can return
	 * nullptr. Will return nullptr if OwningActor is nullptr and, otherwise, if
	 * OwningActor, or it's child actors if bSearchChildActors is true, doesn't
	 * containt a UAGX_RigidBodyComponent name BodyName.
	 */
	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Get the Actor that owns the RigidBody that this reference currently references. Can return
	 * nullptr.
	 */
	AActor* GetOwningActor() const;

	/**
	 * Forget the currently cached body and perform a new search through
	 * OwningActor's components.
	 */
	void UpdateCache() const;

	/**
	 * Forget the currently cached body. Should be called when any of the search
	 * parameters is changed.
	 */
	void InvalidateCache() const;

private:
	mutable UAGX_RigidBodyComponent* Cache;
};
