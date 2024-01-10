// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

struct FAGX_SensorMsgsImage;

class AGXUNREAL_API FAGX_ROS2Utilities
{
public:
	static FAGX_SensorMsgsImage Convert(
		const TArray<FColor>& Image, float TimeStamp, const FIntPoint& Resolution, bool Grayscale);

	static FAGX_SensorMsgsImage Convert(
		const TArray<FFloat16Color>& Image, float TimeStamp, const FIntPoint& Resolution, bool Grayscale);
};
