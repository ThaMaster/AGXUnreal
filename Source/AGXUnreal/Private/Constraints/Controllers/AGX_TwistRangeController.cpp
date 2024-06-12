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

#if 0
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
#endif

namespace
{
	FTwistRangeControllerBarrier* GetTwistRangeBarrier(FAGX_TwistRangeController& Controller)
	{
		return static_cast<FTwistRangeControllerBarrier*>(Controller.GetNative());
	}

	const FTwistRangeControllerBarrier* GetTwistRangeBarrier(
		const FAGX_TwistRangeController& Controller)
	{
		return static_cast<const FTwistRangeControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_TwistRangeController::SetRange(FDoubleInterval InRange)
{
	if (HasNative())
	{
		GetTwistRangeBarrier(*this)->SetRange(InRange);
	}
	Range = FAGX_RealInterval(InRange);
}

void FAGX_TwistRangeController::SetRangeMin(double InMin)
{
	if (HasNative())
	{
		GetTwistRangeBarrier(*this)->SetRangeMin(InMin);
	}
	Range.Min = InMin;
}

void FAGX_TwistRangeController::SetRangeMax(double InMax)
{
	if (HasNative())
	{
		GetTwistRangeBarrier(*this)->SetRangeMax(InMax);
	}
	Range.Max = InMax;
}

void FAGX_TwistRangeController::SetRange(FAGX_RealInterval InRange)
{
	if (HasNative())
	{
		GetTwistRangeBarrier(*this)->SetRange(InRange);
	}
	Range = InRange;
}

FDoubleInterval FAGX_TwistRangeController::GetRange() const
{
	if (HasNative())
	{
		return GetTwistRangeBarrier(*this)->GetRange();
	}
	return Range;
}

double FAGX_TwistRangeController::GetRangeMin() const
{
	if (HasNative())
	{
		return GetTwistRangeBarrier(*this)->GetRangeMin();
	}
	return Range.Min;
}

double FAGX_TwistRangeController::GetRangeMax() const
{
	if (HasNative())
	{
		return GetTwistRangeBarrier(*this)->GetRangeMax();
	}
	return Range.Max;
}

#if 1
void FAGX_TwistRangeController::InitializeBarrier(
	TUniquePtr<FTwistRangeControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}
#else
void FAGX_TwistRangeController::InitializeBarrier(const FTwistRangeControllerBarrier& Barrier)
{
	check(!HasNative());
	Super::InitializeBarrier(Barrier);
	NativeBarrier = Barrier;
	check(HasNative());
}
#endif

#if 0
void FAGX_TwistRangeController::ClearBarrier()
{
	Super::ClearBarrier();
	NativeBarrier = FTwistRangeControllerBarrier();
}
#endif

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

void FAGX_TwistRangeController::UpdateNativePropertiesImpl()
{
	check(HasNative());
	GetTwistRangeBarrier(*this)->SetRange(Range);
}
