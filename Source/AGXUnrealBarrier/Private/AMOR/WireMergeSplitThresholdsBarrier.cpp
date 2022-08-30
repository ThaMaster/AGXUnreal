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

void FWireMergeSplitThresholdsBarrier::SetForcePropagationDecayScale(
	double InForcePropagationDecayScale)
{
	UE_LOG(LogTemp, Warning, TEXT("Oooooooooooooj barrier"));
	check(HasNative());
	using namespace WireMergeSplitThresholds_helpers;
	if (auto Native = CastToWireThresholds(NativeRef->Native, "SetForcePropagationDecayScale"))
	{
		Native->setForcePropagationDecayScale(InForcePropagationDecayScale);
	}
}

double FWireMergeSplitThresholdsBarrier::GetForcePropagationDecayScale() const
{
	check(HasNative());
	using namespace WireMergeSplitThresholds_helpers;
	if (auto Native = CastToWireThresholds(NativeRef->Native, "GetForcePropagationDecayScale"))
	{
		return Native->getForcePropagationDecayScale();
	}

	// Error message printed above.
	return 0.0;
}

void FWireMergeSplitThresholdsBarrier::SetMergeTensionScale(
	double InMergeTensionScale)
{
	check(HasNative());
	using namespace WireMergeSplitThresholds_helpers;
	if (auto Native = CastToWireThresholds(NativeRef->Native, "SetMergeTensionScale"))
	{
		Native->setMergeTensionScale(InMergeTensionScale);
	}
}

double FWireMergeSplitThresholdsBarrier::GetMergeTensionScale() const
{
	check(HasNative());
	using namespace WireMergeSplitThresholds_helpers;
	if (auto Native = CastToWireThresholds(NativeRef->Native, "GetMergeTensionScale"))
	{
		return Native->getMergeTensionScale();
	}

	// Error message printed above.
	return 0.0;
}