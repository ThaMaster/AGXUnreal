// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarScanPoint.generated.h"

USTRUCT(BlueprintType)
struct FAGX_LidarScanPoint
{
	GENERATED_BODY()

	/**
	* The local position of this point [cm].
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector Position;

	/**
	 * The moment in game time that this point was measured [s].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double TimeStamp {0.0};

	/**
	 * The intensity value asociated with this point, in the range [0..1].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	float Intensity {0.0};

};
