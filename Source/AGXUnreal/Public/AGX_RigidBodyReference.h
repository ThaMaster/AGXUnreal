#pragma once

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor;

	UPROPERTY(EditAnywhere, Category = "AGX Dynamice")
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
	UAGX_RigidBodyComponent* GetRigidBody();

	/**
	 * Forget the currently cached body and perform a new search through
	 * OwningActor's components.
	 */
	void UpdateCache();

	/**
	 * Forget the currently cached body. Should be called when any of the search
	 * parameters is changed.
	 */
	void InvalidateCache();

private:
	UAGX_RigidBodyComponent* Cache;
};
