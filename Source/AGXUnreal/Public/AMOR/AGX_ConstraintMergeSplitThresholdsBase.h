// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholdsBase.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholdsBase
	: public UObject
{
	GENERATED_BODY()

public:
	virtual ~UAGX_ConstraintMergeSplitThresholdsBase() = default;
};