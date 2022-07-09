// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ConstraintMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


#include "AGX_ConstraintMergeSplitThresholdsAsset.generated.h"

class UAGX_ConstraintMergeSplitThresholdsInstance;


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholdsAsset
	: public UAGX_ConstraintMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:

private:
	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholdsInstance> Instance;
};