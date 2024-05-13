// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"


class UAGX_RayPatternBase;

struct FAGX_DistanceGaussianNoiseSettings;
struct FAGX_RealInterval;
struct FAGX_Real;

class AGXUNREAL_API FAGX_SensorUtilities
{
public:
	static FAGX_RealInterval GetRangeFrom(EAGX_LidarModel InModel);
	static FAGX_Real GetBeamDivergenceFrom(EAGX_LidarModel InModel);
	static FAGX_Real GetBeamExitRadiusFrom(EAGX_LidarModel InModel);
	static UAGX_RayPatternBase* GetRayPatternFrom(EAGX_LidarModel InModel);
	static bool GetEnableDistanceGaussianNoiseFrom(EAGX_LidarModel InModel);
	static FAGX_DistanceGaussianNoiseSettings GetDistanceGaussianNoiseFrom(
		EAGX_LidarModel InModel);
};

