// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_MergeSplitThresholdsBase.generated.h"

class FMergeSplitThresholdsBarrier;


UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_MergeSplitThresholdsBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void CopyFrom(const FMergeSplitThresholdsBarrier& Barrier)
		PURE_VIRTUAL(UAGX_MergeSplitThresholdsBase::CopyFrom, );
};
