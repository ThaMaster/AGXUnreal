// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"


// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"

FConstraintMergeSplitThresholdsBarrier::FConstraintMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FConstraintMergeSplitThresholdsBarrier::FConstraintMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef> Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FConstraintMergeSplitThresholdsBarrier::~FConstraintMergeSplitThresholdsBarrier()
{
}

void FConstraintMergeSplitThresholdsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSDK::ConstraintMergeSplitThresholds();
}

namespace ConstraintMergeSplitThresholds_helpers
{
	agxSDK::ConstraintMergeSplitThresholds* CastToConstraintThresholds(
		agxSDK::MergeSplitThresholds* Thresholds, const FString& Operation)
	{
		agxSDK::ConstraintMergeSplitThresholds* ConstraintThresholds =
			dynamic_cast<agxSDK::ConstraintMergeSplitThresholds*>(Thresholds);
		if (ConstraintThresholds == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Operation %s failed, could not cast native MergeSplitThresholds to "
					 "ConstraintMergeSplitThresholds."),
				*Operation);
		}

		return ConstraintThresholds;
	}
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredForceRangeDiff(
	double InMaxDesiredForceRangeDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredForceRangeDiff"))
	{
		Native->setMaxDesiredForceRangeDiff(InMaxDesiredForceRangeDiff);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredForceRangeDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredForceRangeDiff"))
	{
		return Native->getMaxDesiredForceRangeDiff();
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredLockAngleDiff(
	double InMaxDesiredLockAngleDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredLockAngleDiff"))
	{
		Native->setMaxDesiredLockAngleDiff(InMaxDesiredLockAngleDiff);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredLockAngleDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredLockAngleDiff"))
	{
		return Native->getMaxDesiredLockAngleDiff();
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredRangeAngleDiff(
	double InMaxDesiredRangeAngleDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredRangeAngleDiff"))
	{
		Native->setMaxDesiredRangeAngleDiff(InMaxDesiredRangeAngleDiff);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredRangeAngleDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredRangeAngleDiff"))
	{
		return Native->getMaxDesiredRangeAngleDiff();
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredSpeedDiff(
	double InMaxDesiredSpeedDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredSpeedDiff"))
	{
		Native->setMaxDesiredSpeedDiff(InMaxDesiredSpeedDiff);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredSpeedDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredSpeedDiff"))
	{
		return Native->getMaxDesiredSpeedDiff();
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxRelativeSpeed(
	double InMaxRelativeSpeed)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxRelativeSpeed"))
	{
		Native->setMaxRelativeSpeed(InMaxRelativeSpeed);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxRelativeSpeed() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxRelativeSpeed"))
	{
		return Native->getMaxRelativeSpeed();
	}

	// Error message printed above.
	return 0.0;
}
