// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholdsAsset.generated.h"

class UAGX_ShapeContactMergeSplitThresholdsInstance;


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholdsAsset
	: public UAGX_ShapeContactMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_ShapeContactMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

private:
	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholdsInstance> Instance;
};