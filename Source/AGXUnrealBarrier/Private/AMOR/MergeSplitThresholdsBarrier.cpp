// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/MergeSplitThresholdsBarrier.h"


// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"



FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier()
	: NativeRef {new FMergeSplitThresholdsRef}
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native)
	: NativeRef(std::move(Native))
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(FMergeSplitThresholdsBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
}

FMergeSplitThresholdsBarrier::~FMergeSplitThresholdsBarrier()
{
}

bool FMergeSplitThresholdsBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative()
{
	return NativeRef.get();
}

const FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative() const
{
	return NativeRef.get();
}

void FMergeSplitThresholdsBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
