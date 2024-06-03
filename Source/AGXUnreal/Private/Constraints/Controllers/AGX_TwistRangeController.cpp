// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TwistRangeController.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_TwistRangeController::FAGX_TwistRangeController()
{
	bEnable = false;
}

FAGX_TwistRangeController::~FAGX_TwistRangeController()
{
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

void FAGX_TwistRangeController::SetRange(FDoubleInterval InRange){
	if (HasNative())
	{
		NativeBarrier.SetRange(InRange);
	}
	Range = FAGX_RealInterval(InRange);
}


void FAGX_TwistRangeController::SetRange(FAGX_RealInterval InRange)
{
	if (HasNative())
	{
		NativeBarrier.SetRange(InRange);
	}
	Range = InRange;
}

FDoubleInterval FAGX_TwistRangeController::GetRange() const
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
	Super::InitializeBarrier(Barrier);
	NativeBarrier = Barrier;
	check(HasNative());
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

void FAGX_TwistRangeController::UpdateNativeProperties()
{
	check(HasNative());
	Super::UpdateNativeProperties();
	NativeBarrier.SetRange(Range);
}
