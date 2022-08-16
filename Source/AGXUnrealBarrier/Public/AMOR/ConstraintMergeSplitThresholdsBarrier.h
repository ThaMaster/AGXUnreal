// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class AGXUNREALBARRIER_API FConstraintMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FConstraintMergeSplitThresholdsBarrier();
	FConstraintMergeSplitThresholdsBarrier(FConstraintMergeSplitThresholdsBarrier&& Other) = default;
	FConstraintMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef> Native);
	~FConstraintMergeSplitThresholdsBarrier();

	void AllocateNative();

private:
	FConstraintMergeSplitThresholdsBarrier(const FConstraintMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FConstraintMergeSplitThresholdsBarrier&) = delete;
};