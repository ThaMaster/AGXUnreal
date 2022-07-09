// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_WireMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


#include "AGX_WireMergeSplitThresholdsAsset.generated.h"

class UAGX_WireMergeSplitThresholdsInstance;


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_WireMergeSplitThresholdsAsset
	: public UAGX_WireMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:

private:
	TWeakObjectPtr<UAGX_WireMergeSplitThresholdsInstance> Instance;
};