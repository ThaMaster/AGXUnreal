// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireParameterController.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_BarrierUtilities.h"
#include "Wire/WireParameterControllerBarrier.h"

void FAGX_WireParameterController::SetBarrier(const FWireParameterControllerBarrier& InBarrier)
{
	NativeBarrier = InBarrier;
}

AGX_BARRIER_SET_GET_PROPERTY(
	FAGX_WireParameterController, double, MaximumContactMovementOneTimestep)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, MinimumDistanceBetweenNodes);
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, ScaleConstant)

void FAGX_WireParameterController::WritePropertiesToNative()
{
	NativeBarrier.SetMaximumContactMovementOneTimestep(MaximumContactMovementOneTimestep);
	NativeBarrier.SetMinimumDistanceBetweenNodes(MinimumDistanceBetweenNodes);
	NativeBarrier.SetScaleConstant(ScaleConstant);
}

bool FAGX_WireParameterController::HasNative() const
{
	return NativeBarrier.HasNative();
}
