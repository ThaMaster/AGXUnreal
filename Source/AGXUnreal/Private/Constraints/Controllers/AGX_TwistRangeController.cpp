// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TwistRangeController.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_TwistRangeController::FAGX_TwistRangeController()
{
}

FAGX_TwistRangeController::~FAGX_TwistRangeController()
{
}

FAGX_TwistRangeController& FAGX_TwistRangeController::operator=(
	const FAGX_TwistRangeController& Other)
{
	bEnable = Other.bEnable;
	Range = Other.Range;
	return *this;
}

void FAGX_TwistRangeController::SetEnable(bool bInEnable)
{
	if (HasNative())
	{
		NativeBarrier.SetEnable(bInEnable);
	}
	bEnable = bInEnable;
}

bool FAGX_TwistRangeController::GetEnable() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnable();
	}
	return bEnable;
}

bool FAGX_TwistRangeController::HasNative() const
{
	return NativeBarrier.HasNative();
}

FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void FAGX_TwistRangeController::SetRange(FAGX_RealInterval InRange)
{
	if (HasNative())
	{
		NativeBarrier.SetRange(InRange);
	}
	Range = InRange;
}

FAGX_RealInterval FAGX_TwistRangeController::GetRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRange();
	}
	return Range;
}

void FAGX_TwistRangeController::InitializeBarrier(const FTwistRangeControllerBarrier& Barrier)
{
	check(!HasNative());
	NativeBarrier = Barrier;
	check(HasNative());
}

void FAGX_TwistRangeController::CopyFrom(
	const FTwistRangeControllerBarrier& Source,
	TArray<FAGX_TwistRangeController*>& ArchetypeInstances, bool bForceOverwriteInstances)
{
	// TODO Figure out how this should be, if we decide to have a base class for Twist Range
	// Controller. Otherwise remove this bit.
#if 0
	TArray<FAGX_ConstraintController*> BaseInstances(ArchetypeInstances);
	Super::CopyFrom(Source, BaseInstances, bForceOverwriteInstances);
#endif

	const bool bEnableBarrier = Source.GetEnable();
	const FAGX_RealInterval RangeBarrier = Source.GetRange();

	for (auto Instance : ArchetypeInstances)
	{
		if (Instance == nullptr)
		{
			continue;
		}

		if (bForceOverwriteInstances || Instance->bEnable == bEnable)
		{
			Instance->bEnable = bEnableBarrier;
		}
		if (bForceOverwriteInstances || Instance->Range == Range)
		{
			Instance->Range = RangeBarrier;
		}
	}

	bEnable = bEnableBarrier;
	Range = RangeBarrier;
}

void FAGX_TwistRangeController::UpdateNativeProperties()
{
	check(HasNative());
	NativeBarrier.SetEnable(bEnable);
	NativeBarrier.SetRange(Range);
}
