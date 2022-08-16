// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireMergeSplitThresholdsBase.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_WireMergeSplitThresholdsBase
	: public UObject
{
	GENERATED_BODY()

public:
	virtual ~UAGX_WireMergeSplitThresholdsBase() = default;

	virtual UAGX_WireMergeSplitThresholdsBase* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_WireMergeSplitThresholdsBase::GetOrCreateInstance,
					 return nullptr;);
};