// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/WireMergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"


FWireMergeSplitThresholdsBarrier::FWireMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FWireMergeSplitThresholdsBarrier::FWireMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef> Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FWireMergeSplitThresholdsBarrier::~FWireMergeSplitThresholdsBarrier()
{
}

void FWireMergeSplitThresholdsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSDK::WireMergeSplitThresholds();
}

namespace WireMergeSplitThresholds_helpers
{
	agxSDK::WireMergeSplitThresholds* CastToWireThresholds(
		agxSDK::MergeSplitThresholds* Thresholds, const FString& Operation)
	{
		agxSDK::WireMergeSplitThresholds* WireThresholds =
			dynamic_cast<agxSDK::WireMergeSplitThresholds*>(Thresholds);
		if (WireThresholds == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Operation %s failed, could not cast native MergeSplitThresholds to "
					 "WireMergeSplitThresholds."),
				*Operation);
		}

		return WireThresholds;
	}
}
