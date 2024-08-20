// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactEnums.h"
#include "Contacts/AGX_ShapeContact.h"

// Unreal Engine includes
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_ContactEventListenerComponent.generated.h"

/**
 * Provides access to Shape Contacts before contact pruning and Contac Constraint generation.
 * Makes it possible to manipulate and disable contacts.
 *
 * Listen to contacts either by creating a Blueprint class inheriting from AGX Contact Event
 * Listener or by adding an instance of the C++ Component to an Actor and bind to the Impact,
 * Contact, and/or Separation events.
 */
UCLASS(
	BlueprintType, Blueprintable, Category = "AGX", ClassGroup = "AGX",
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ContactEventListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public: // Blueprint Implementable Events.
	/**
	 * Callback that is called when AGX Dynamics detects an impact between two Shapes.
	 *
	 * @param Time The current time stamp.
	 * @param ShapeContact The Shape Contact that was reported.
	 * @return What to do with the Shape Contact.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AGX Contact Event Listener")
	EAGX_KeepContactPolicy Impact(double Time, const FAGX_ShapeContact& ShapeContact);

	/**
	 * Callback that is called when AGX Dynamics detects a contact between two Shapes.
	 *
	 * @param Time The current time stamp.
	 * @param ShapeContact The Shape Contact that was reported.
	 * @return What to do with the Shape Contact.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AGX Contact Event Listener")
	EAGX_KeepContactPolicy Contact(double Time, const FAGX_ShapeContact& ShapeContact);

	/**
	 * The intention is that this should be called when AGX Dynamics detects a separation between
	 * two Shapes, but I was not able to finish this in time.
	 *
	 * @param Time The current AGX Dynamics time stamp.
	 * @param FirstShape One of the Shapes that is no longer in contact with the other Shape.
	 * @param SecondShape The other Shape, that is no longer in contact with the first Shape.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AGX Contact Event Listener")
	void Separation(
		double Time, const UAGX_ShapeComponent* FirstShape, UAGX_ShapeComponent* SecondShape);

public: // Member function overrides.
	//~ Begin UActorComponent interface.

	/// Setup the AGX Dynamics Contact Event Listener that will call the internal callbacks.
	virtual void BeginPlay() override;

	//~ End UActorComponent interface.

private: // Internal callbacks. Pointers to these are passed to the AGX Dynamics Contact Event
		 // Listener.
	EAGX_KeepContactPolicy ImpactCallback(double Time, FShapeContactBarrier& ShapeContact);
	EAGX_KeepContactPolicy ContactCallback(double Time, FShapeContactBarrier& ShapeContact);
	void SeparationCallback(double Time, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape);

};
