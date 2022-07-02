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
