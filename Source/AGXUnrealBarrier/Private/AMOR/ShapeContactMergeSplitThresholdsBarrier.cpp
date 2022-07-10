// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"


FShapeContactMergeSplitThresholdsBarrier::FShapeContactMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FShapeContactMergeSplitThresholdsBarrier::FShapeContactMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef> Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FShapeContactMergeSplitThresholdsBarrier::~FShapeContactMergeSplitThresholdsBarrier()
{
}

void FShapeContactMergeSplitThresholdsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSDK::GeometryContactMergeSplitThresholds();
}

namespace ShapeContactMergeSplitThresholds_helpers
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
					 "ShapeContactMergeSplitThresholds."), *Operation);
		}

		return GeomContThresholds;
	}
}

