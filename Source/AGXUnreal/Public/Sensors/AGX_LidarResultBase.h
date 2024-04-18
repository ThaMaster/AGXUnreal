// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarResultBase.generated.h"

class FLidarResultBarrier;
class UAGX_LidarSensorComponent;

USTRUCT(BlueprintType, Meta = (HiddenByDefault))
struct AGXUNREAL_API FAGX_LidarResultBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarResultBase() = default;

	/**
	 * Enables or disables removal of point misses, i.e. makes the result dense if set to true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	bool bEnableRemovePointsMisses {true};

	/**
	 * Enables distance gaussian noise, adding an individual distance error to each measurements
	 * of Position.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	bool bEnableDistanceGaussianNoise {false};

	/**
	* Determines the distance noise characteristics. The standard deviation is calculated as
	* s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	*/
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableDistanceGaussianNoise"))
	FAGX_DistanceGaussianNoiseSettings DistanceNoiseSettings;

	virtual bool HasNative() const PURE_VIRTUAL(FAGX_LidarResultBase::HasNative, return false;);

	virtual FLidarResultBarrier* GetOrCreateNative()
		PURE_VIRTUAL(FAGX_LidarResultBase::GetOrCreateNative, return nullptr;);

	virtual const FLidarResultBarrier* GetNative() const
		PURE_VIRTUAL(FAGX_LidarResultBase::GetNative, return nullptr;);

	// Making UAGX_LidarSensorComponent::AddResult Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	bool AddTo(UAGX_LidarSensorComponent* Lidar);

	void PostAllocateNative(FLidarResultBarrier* Native);
};
