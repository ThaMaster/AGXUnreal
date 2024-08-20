// Copyright 2024, Algoryx Simulation AB.

#include "Contacts/AGX_ContactEventListenerComponent.h"

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactListenerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"

void UAGX_ContactEventListenerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create an AGX Dynamics Contact Event Listener that calls our OnImpact, OnContact, and
	// OnSeparation member functions via lambda functions.
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	FSimulationBarrier* SimulationBarrier = Simulation->GetNative();
	CreateContactEventListener(
		*SimulationBarrier,
		[this](double Time, FShapeContactBarrier& ShapeContact)
		{ return ImpactCallback(Time, ShapeContact); },
		[this](double Time, FShapeContactBarrier& ShapeContact)
		{ return ContactCallback(Time, ShapeContact); },
		[this](double Time, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape)
		{ SeparationCallback(Time, FirstShape, SecondShape); });
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::ImpactCallback(
	double TimeStamp, FShapeContactBarrier& ContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function and the delegate.
	FAGX_ShapeContact Contact(ContactBarrier);
	EAGX_KeepContactPolicy Policy = Impact(TimeStamp, Contact);
	if (Policy != EAGX_KeepContactPolicy::RemoveContactImmediately)
	{
		FAGX_KeepContactPolicy PolicyHandle {&Policy};
		OnImpact.Broadcast(TimeStamp, Contact, PolicyHandle);
	}
	return Policy;
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::ContactCallback(
	double TimeStamp, FShapeContactBarrier& ContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function and the delegate
	FAGX_ShapeContact Contact(ContactBarrier);
	EAGX_KeepContactPolicy Policy = Impact(TimeStamp, Contact);
	if (Policy != EAGX_KeepContactPolicy::RemoveContactImmediately)
	{
		FAGX_KeepContactPolicy PolicyHandle {&Policy};
		OnContact.Broadcast(TimeStamp, Contact, PolicyHandle);
	}
	return Policy;
}

void UAGX_ContactEventListenerComponent::SeparationCallback(
	double Time, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape)
{
	// TODO Implement UAGX_ContactEventListenerComponent::SeparationCallback.
}


EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::Impact_Implementation(double Time, const FAGX_ShapeContact& ShapeContact)
{
	return EAGX_KeepContactPolicy::KeepContact;
}


EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::Contact_Implementation(double Time, const FAGX_ShapeContact& ShapeContact)
{
	return EAGX_KeepContactPolicy::KeepContact;
}


void UAGX_ContactEventListenerComponent::Separation_Implementation(
		double Time, const UAGX_ShapeComponent* FirstShape, UAGX_ShapeComponent* SecondShape)
{
}
