// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/GeometryContactMergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"


FGeometryContactMergeSplitThresholdsBarrier::FGeometryContactMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FGeometryContactMergeSplitThresholdsBarrier::FGeometryContactMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef> Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FGeometryContactMergeSplitThresholdsBarrier::~FGeometryContactMergeSplitThresholdsBarrier()
{
}

void FGeometryContactMergeSplitThresholdsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSDK::GeometryContactMergeSplitThresholds();
}

namespace GeometryContactMergeSplitThresholds_helpers
{
	agxSDK::GeometryContactMergeSplitThresholds* CastToGeometryContactThresholds(
		agxSDK::MergeSplitThresholds* Thresholds, const FString& Operation)
	{
		agxSDK::GeometryContactMergeSplitThresholds* GeomContThresholds =
			dynamic_cast<agxSDK::GeometryContactMergeSplitThresholds*>(Thresholds);
		if (GeomContThresholds == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Operation %s failed, could not cast native MergeSplitThresholds to "
					 "GeometryContactMergeSplitThresholds."), *Operation);
		}

		return GeomContThresholds;
	}
}

