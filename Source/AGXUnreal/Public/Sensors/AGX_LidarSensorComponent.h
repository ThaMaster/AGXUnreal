// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Sensors/LidarBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

class UTextureRenderTarget2D;

struct FAGX_SensorMsgsPointCloud2;

#include "AGX_LidarSensorComponent.generated.h"


/**
 * Lidar Sensor Component, allowing to create point cluds at runtime.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_LidarSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_LidarSensorComponent();

	/**
	 * The maximum range of the Lidar Sensor [cm].
	 * Objects farther away than this value will not be detected by this Lidar Sensor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	FAGX_Real Range {20000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void Step();

	bool HasNative() const;
	FLidarBarrier* GetOrCreateNative();
	FLidarBarrier* GetNative();
	const FLidarBarrier* GetNative() const;

	void GetResultTest(); // Test function, do not merge!!

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

private:
	FLidarBarrier NativeBarrier;
};
