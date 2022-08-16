// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class AGXUNREALBARRIER_API FShapeContactMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FShapeContactMergeSplitThresholdsBarrier();
	FShapeContactMergeSplitThresholdsBarrier(FShapeContactMergeSplitThresholdsBarrier&& Other) = default;
	FShapeContactMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef> Native);
	~FShapeContactMergeSplitThresholdsBarrier();

	void AllocateNative();

private:
	FShapeContactMergeSplitThresholdsBarrier(const FShapeContactMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FShapeContactMergeSplitThresholdsBarrier&) = delete;
};
