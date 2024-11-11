// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLX_SignalHandlerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"

UPLX_SignalHandlerComponent::UPLX_SignalHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace PLX_SignalHandlerComponent_helpers
{
	TArray<FConstraintBarrier*> CollectConstraintBarriers(AActor* Owner)
	{
		if (Owner == nullptr)
			return TArray<FConstraintBarrier*>();

		TArray<UAGX_ConstraintComponent*> ConstraintsInThisActor =
			FAGX_ObjectUtilities::Filter<UAGX_ConstraintComponent>(Owner->GetComponents());
		TArray<FConstraintBarrier*> ConstraintBarriers;
		for (UAGX_ConstraintComponent* Constraint : ConstraintsInThisActor)
		{
			if (auto CBarrier = Constraint->GetOrCreateNative())
				ConstraintBarriers.Add(CBarrier);
		}

		return ConstraintBarriers;
	}
}

void UPLX_SignalHandlerComponent::BeginPlay()
{
	using namespace PLX_SignalHandlerComponent_helpers;
	Super::BeginPlay();

	auto Sim = UAGX_Simulation::GetFrom(this);
	auto SimulationBarrier = Sim != nullptr ? Sim->GetNative() : nullptr;
	if (SimulationBarrier == nullptr)
	{
		// todo: log warning.
		return;
	}

	// Collect all Constraints in the same AActor as us.
	TArray<FConstraintBarrier*> ConstraintBarriers = CollectConstraintBarriers(GetOwner());

	// Initialize SignalHandler in Barrier module.
	SignalHandler.Init(*SimulationBarrier, ConstraintBarriers);
}

