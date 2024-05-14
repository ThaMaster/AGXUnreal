// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_LidarOutputTypes.generated.h"


USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_LidarOutputPositionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	FVector Position {FVector::ZeroVector};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_LidarOutputPositionIntensityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	FVector Position {FVector::ZeroVector};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	double Intensity;
};
