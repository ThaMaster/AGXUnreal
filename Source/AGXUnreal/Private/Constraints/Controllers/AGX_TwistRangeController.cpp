// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TwistRangeController.h"

// AGX Dynamics for Unreal includes.

// Special member functions.

FAGX_TwistRangeController::FAGX_TwistRangeController()
{
	bEnable = false;
}

FAGX_TwistRangeController::~FAGX_TwistRangeController()
{
}

// Properties.

void FAGX_TwistRangeController::SetRange(double InRangeMin, double InRangeMax)
{
	if (HasNative())
	{
		Barrier.SetRange(FDoubleInterval(InRangeMin, InRangeMax));
	}
	Range = FAGX_RealInterval {InRangeMin, InRangeMax};
}

void FAGX_TwistRangeController::SetRange(FAGX_RealInterval InRange)
{
	SetRange(InRange.Min, InRange.Max);
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
	Super::UpdateNativeProperties();
	Barrier.SetRange(Range);
}

// Native management.

bool FAGX_TwistRangeController::HasNative() const
{
	check(Super::HasNative() == Barrier.HasNative());
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
	Super::InitializeBarrier(InBarrier);
	Barrier = InBarrier;
}

void FAGX_TwistRangeController::CopyFrom(
	const FTwistRangeControllerBarrier& Source,
	TArray<FAGX_TwistRangeController*>& ArchetypeInstances, bool bForceOverwriteInstances)
{
	TArray<FAGX_ElementaryConstraint*> BaseInstances(ArchetypeInstances);
	Super::CopyFrom(Source, BaseInstances, bForceOverwriteInstances);

	const FAGX_RealInterval RangeBarrier = Source.GetRange();

	for (auto Instance : ArchetypeInstances)
	{
		if (Instance == nullptr)
		{
			continue;
		}

		if (bForceOverwriteInstances || Instance->Range == Range)
		{
			Instance->Range = RangeBarrier;
		}
	}

	Range = RangeBarrier;
}
