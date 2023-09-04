// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal include.
#include "AGX_Frame.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Edge.generated.h"

class USceneComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_Edge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Edge")
	FAGX_Frame Start;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Edge")
	FAGX_Frame End;

	FTwoVectors GetLocationsRelativeTo(USceneComponent* Component);
};
