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
	double Time, FShapeContactBarrier& ShapeContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function.
	FAGX_ShapeContact ShapeContact(std::move(ShapeContactBarrier));
	return Impact(Time, ShapeContact);
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::ContactCallback(
	double Time, FShapeContactBarrier& ShapeContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function.
	FAGX_ShapeContact ShapeContact(std::move(ShapeContactBarrier));
	return Contact(Time, ShapeContact);
}

void UAGX_ContactEventListenerComponent::SeparationCallback(
	double Time, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape)
{
	// This is the part I was not able to implement in time. We need a way to find the AGX Shape
	// Component that corresponds to the given FAnyShapeBarriers.
}
