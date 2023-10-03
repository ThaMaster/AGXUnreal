// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_ROS2Messages.generated.h"


USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_ROS2Message
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsFloat32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float data{0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 data {0};
};
