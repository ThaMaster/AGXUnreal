// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
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
	 * The minimum and maximum range of the Lidar Sensor [cm].
	 * Objects outside this range will not be detected by this Lidar Sensor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	FAGX_RealInterval Range {0.0, 20000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void SetRange(FAGX_RealInterval InRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	FAGX_RealInterval GetRange() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void Step();

	bool HasNative() const;
	FLidarBarrier* GetOrCreateNative();
	FLidarBarrier* GetNative();
	const FLidarBarrier* GetNative() const;

	void GetResultTest(); // Test function, do not merge!!

#if WITH_EDITOR
	//~ Begin UActorComponent Interface
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	//~ End UActorComponent Interface

	// ~Begin UObject interface.
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
	// ~End UObject interface.
#endif

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void UpdateNativeProperties();

	FLidarBarrier NativeBarrier;
};
