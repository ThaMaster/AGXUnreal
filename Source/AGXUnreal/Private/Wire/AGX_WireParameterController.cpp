// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireParameterController.h"

// AGX Dynamics for Unreal includes.
#include "Wire/WireParameterControllerBarrier.h"

void FAGX_WireParameterController::SetBarrier(const FWireParameterControllerBarrier& InBarrier)
{
	Barrier = InBarrier;
}

void FAGX_WireParameterController::SetScaleConstant(double InScaleConstant)
{
	if (HasNative())
	{
		Barrier.SetScaleConstant(InScaleConstant);
	}
	ScaleConstant = InScaleConstant;
}

double FAGX_WireParameterController::GetScaleConstant() const
{
	if (HasNative())
	{
		return Barrier.GetScaleConstant();
	}
	else
	{
		return ScaleConstant;
	}
}

void FAGX_WireParameterController::WritePropertiesToNative()
{
	Barrier.SetScaleConstant(ScaleConstant);
}

bool FAGX_WireParameterController::HasNative() const
{
	return Barrier.HasNative();
}
