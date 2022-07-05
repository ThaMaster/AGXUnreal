// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_GeometryContactMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


#include "AGX_GeometryContactMergeSplitThresholdsAsset.generated.h"

class UAGX_GeometryContactMergeSplitThresholdsInstance;


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_GeometryContactMergeSplitThresholdsAsset
	: public UAGX_GeometryContactMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:

private:
	TWeakObjectPtr<UAGX_GeometryContactMergeSplitThresholdsInstance> Instance;
};