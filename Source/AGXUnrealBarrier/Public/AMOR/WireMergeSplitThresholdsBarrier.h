// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class AGXUNREALBARRIER_API FWireMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FWireMergeSplitThresholdsBarrier();
	FWireMergeSplitThresholdsBarrier(FWireMergeSplitThresholdsBarrier&& Other) = default;
	FWireMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef> Native);
	virtual ~FWireMergeSplitThresholdsBarrier();

	void AllocateNative();

private:
	FWireMergeSplitThresholdsBarrier(const FWireMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FWireMergeSplitThresholdsBarrier&) = delete;
};