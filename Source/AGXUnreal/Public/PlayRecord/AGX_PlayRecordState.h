// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


#include "AGX_PlayRecordState.generated.h"

USTRUCT()
struct FAGX_PlayRecordState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Play Record")
	TArray<FAGX_Real> Values;
};
