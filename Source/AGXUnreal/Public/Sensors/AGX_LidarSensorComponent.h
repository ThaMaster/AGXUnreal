// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Sensors/AGX_CustomPatternFetcher.h"
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/LidarBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_LidarSensorComponent.generated.h"

class UTextureRenderTarget2D;
struct FAGX_SensorMsgsPointCloud2;

DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FTransform>, FOnFetchRayTransforms);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(
	FAGX_CustomPatternInterval, FOnFetchNextPatternInterval, double, TimeStamp);

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

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetRange(FAGX_RealInterval InRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	FAGX_RealInterval GetRange() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_LidarScanPattern ScanPattern {EAGX_LidarScanPattern::HorizontalSweep};

	/**
	 * Delegate that has to be assigned (bound to) by the user to support custom scan pattern.
	 * Only used if the ScanPattern is set to Custom.
	 * Should return all ray transforms (in global coordinates) for the whole scan pattern.
	 * This delegate is called only if no ray transforms has previously been returned by the funcion
	 * provided by the user, i.e. under normal conditions, it is called only once.
	 * The FetchNextPatternInterval is called each Step() and determines what part of the scan
	 * pattern to use next, see OnFetchNextPatternInterval.
	 * The signature of the function assigned must be: TArray<FTransform> MyFunc().
	 */
	UPROPERTY(
		BlueprintReadWrite, Category = "AGX Lidar",
		Meta = (EditCondition = "ScanPattern == EAGX_LidarScanPattern::Custom"))
	FOnFetchRayTransforms OnFetchRayTransforms;

	/**
	 * Delegate that has to be assigned (bound to) by the user to support custom scan pattern.
	 * Only used if the ScanPattern is set to Custom.
	 * Should return the next AGX Custom Pattern Interval to use.
	 * This delegate is called each Step() and determines what part of the total scan pattern to use
	 * in that Step(). See also OnFetchRayTransforms.
	 * The signature of the function assigned must be: FAGX_CustomPatternInterval
	 * MyFunc(double TimeStamp).
	 */
	UPROPERTY(
		BlueprintReadWrite, Category = "AGX Lidar",
		Meta = (EditCondition = "ScanPattern == EAGX_LidarScanPattern::Custom"))
	FOnFetchNextPatternInterval OnFetchNextPatternInterval;

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
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

	friend class FAGX_CustomPatternFetcher;

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void UpdateNativeProperties();

	TArray<FTransform> FetchRayTransforms();
	FAGX_CustomPatternInterval FetchNextInterval();

	FAGX_CustomPatternFetcher PatternFetcher;
	FLidarBarrier NativeBarrier;
};
