// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TwistRangeController.h"

#if 1

// AGX Dynamics for Unreal includes.

// Special member functions.

FAGX_TwistRangeController::FAGX_TwistRangeController()
{
	bEnabled = false;
}

FAGX_TwistRangeController::~FAGX_TwistRangeController()
{
}

// Properties.

void FAGX_TwistRangeController::SetEnabled(bool bInEnabled)
{
	if (HasNative())
	{
		Barrier.SetEnabled(bInEnabled);
	}
	bEnabled = bInEnabled;
}

bool FAGX_TwistRangeController::GetEnabled() const
{
	if (HasNative())
	{
		return Barrier.GetEnabled();
	}
	return bEnabled;
}

void FAGX_TwistRangeController::SetRange(double InRangeMin, double InRangeMax)
{
	if (HasNative())
	{
		Barrier.SetRange(FDoubleInterval(InRangeMin, InRangeMax));
	}
	Range = FAGX_RealInterval {InRangeMin, InRangeMax};
}

double FAGX_TwistRangeController::GetRangeMin() const
{
	if (HasNative())
	{
		return Barrier.GetRange().Min;
	}
	return Range.Min;
}

double FAGX_TwistRangeController::GetRangeMax() const
{
	if (HasNative())
	{
		return Barrier.GetRange().Max;
	}
	return Range.Max;
}

void FAGX_TwistRangeController::UpdateNativeProperties()
{
	check(HasNative());

	// TODO Enable this once we inherit from Elementary Constraint.
	// Super::UpdateNativeProperties();

	// TODO Remove this once we inherit from Elementary Constraint.
	Barrier.SetEnabled(bEnabled);

	Barrier.SetRange(Range);
}

// Native management.

bool FAGX_TwistRangeController::HasNative() const
{
	return Barrier.HasNative();
}

FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

const FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

void FAGX_TwistRangeController::InitializeBarrier(const FTwistRangeControllerBarrier& InBarrier)
{
	check(!HasNative());

	// TODO Enable this once we inherit from Elementary Constraint.
	// Super::InializeBarrier();

	Barrier = InBarrier;
}

#endif
