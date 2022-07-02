// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class AGXUNREALBARRIER_API FGeometryContactMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FGeometryContactMergeSplitThresholdsBarrier();
	FGeometryContactMergeSplitThresholdsBarrier(FGeometryContactMergeSplitThresholdsBarrier&& Other) = default;
	FGeometryContactMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef> Native);
	virtual ~FGeometryContactMergeSplitThresholdsBarrier();

	void AllocateNative();

private:
	FGeometryContactMergeSplitThresholdsBarrier(const FGeometryContactMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FGeometryContactMergeSplitThresholdsBarrier&) = delete;
};
