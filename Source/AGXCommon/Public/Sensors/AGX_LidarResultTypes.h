// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_LidarResultTypes.generated.h"


USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_LidarResultPositionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	FVector Position;
};
