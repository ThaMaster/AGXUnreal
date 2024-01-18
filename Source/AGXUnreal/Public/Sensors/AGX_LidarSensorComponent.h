// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarScanPoint.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

class UTextureRenderTarget2D;

#include "AGX_LidarSensorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPointCloudDataOutput, const TArray<FAGX_LidarScanPoint>&, Points);

/**
 * EXPERIMENTAL
 *
 * Lidar Sensor Component, allowing to create point cluds at runtime.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_LidarSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_LidarSensorComponent();

	/**
	 * Determines whether the Lidar Sensor should perform scanning and output data automatically
	 * (according to frequencies set) or if it should be passive and only perform these operations
	 * when explicitly asked to, e.g. using the PerformScan function.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	TEnumAsByte<enum EAGX_LidarExecutonMode> ExecutionMode {EAGX_LidarExecutonMode::Auto};

	/**
	 * Determines how often the total scan cycle is run, i.e. how often the total scan pattern of
	 * the Lidar Sensor is covered [Hz].
	 * This is separate from the OutputFrequency which determines how often the Lidar Sensor outputs
	 * data.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Lidar",
		Meta = (EditCondition = "ExecutionMode == EAGX_LidarExecutonMode::Auto", ClampMin = "0.0"))
	double ScanFrequency {10};

	/**
	 * Determines how often the Lidar Sensor outputs point cloud data [Hz].
	 * This  is done through the FOnPointCloudDataOutput delegate.
	 * This frequency must be at least as high as the ScanFrequency.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Lidar",
		Meta = (EditCondition = "ExecutionMode == EAGX_LidarExecutonMode::Auto", ClampMin = "0.0"))
	double OutputFrequency {20};

	/**
	 * Delegate that is executed each time the Lidar Sensor outputs point cloud data.
	 * How often this delegate is executed is controlled by the OutputFrequency.
	 * Users may bind to this delegate in order to get a callback.
	 *
	 * Important: The point cloud data passed via this delegate is only valid during the delegate's
	 * execution. Any attempt to store these values must be from copying them by value.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnPointCloudDataOutput PointCloudDataOutput;

	/**
	 * Sampling type, i.e. how the laser rays are generated.
	 * Currently, only CPU rays are supported.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	TEnumAsByte<enum EAGX_LidarSamplingType> SamplingType {EAGX_LidarSamplingType::CPU};

	/**
	 * Field of View (FOV) of the Lidar Sensor [deg].
	 * The first element is the horizontal FOV, and the second vertical FOV.
	 * Setting FOV to zero will essentially represent a 1D (single ray) Lidar.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	FVector2D FOV {120.0, 30.0};

	/**
	 * Resolution, the smallest angle between two laser rays [deg].
	 * The first element is the horizontal FOV, and the second vertical FOV.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	FVector2D Resolution {0.25, 0.25};

	/**
	 * The maximum range of the Lidar Sensor [cm].
	 * Objects farther away than this value will not be detected by this Lidar Sensor.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar", meta = (ClampMin = "0.0"))
	double Range {20000.0};

	/**
	 * Determines whether an intensity value is calculated or not. If set to false, the value zero
	 * is written instead. Using this functionality comes with a performance cost.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	bool bCalculateIntensity {true};

	/**
	 * Determines whether the scanned points should be drawn on screen or not. Using this
	 * functionality comes with a performance cost.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	bool bDebugRenderPoints {true};

	/**
	 * Manually start a Lidar scan.
	 * The scanned point cloud data is broadcasted via the PointCloudDataOutput delegate which users
	 * of this Lidar Sensor may bind to in order to access the data. This delegate is always
	 * executed once upon calling this function, as long as some points were measured.
	 *
	 * This function should only be called when ExecutionMode is set to Manual, and will do nothing
	 * if it is not.
	 *
	 * (Optional) FractionStart and FractionEnd determines what part of the complete scan cycle
	 * should be scanned, i.e. how much of the complete scan pattern should be scanned. To complete
	 * a full scan cycle, the FractionStart should be set to zero and FractionEnd should be set to
	 * one. These parameters will both be clamped to [0..1].
	 *
	 * (Optional) FOVWindowX and FOVWindowY can be used to sample only a sub window of the whole
	 * Lidar Sensor's FOV [deg].
	 * In other words, it can be used to specify a sub-section of the
	 * Lidar Sensor's FOV to be considered during the scan, and any area outside this sub-section
	 * are not scanned. This can increase the performance.
	 * The X component in each FOVWindow represents the lower bound and the Y component the upper
	 * bound.
	 * If left unchanged, the whole Lidar Sensor FOV will be considered. Using the FOV window
	 * does not affect the apparent scale of the scan pattern, it can be seen as a mask that simply
	 * discards points outside this FOV window.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void RequestManualScan(
		double FractionStart = 0.0, double FractionEnd = 1.0,
		FVector2D FOVWindowX = FVector2D::ZeroVector, FVector2D FOVWindowY = FVector2D::ZeroVector);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

private:
	struct FAGX_LidarState
	{
		double ElapsedTime {0.0};
		double ElapsedTimePrev {0.0}; // Elapsed time at last tick.
		double ScanCycleDuration {0.0}; // Always 1 / ScanFrequency.
		double OutputCycleDuration {0.0}; // Always 1 / OutputFrequency.
		double CurrentScanCycleStartTime {0.0};
		double CurrentOutputCycleStartTime {0.0};
	};

	FAGX_LidarState LidarState;
	bool bIsValid {false};

	// Buffer for storing scan data until the next data output is run.
	TArray<FAGX_LidarScanPoint> Buffer;

	bool CheckValid() const;
	void UpdateElapsedTime();
	void ScanAutoCPU();
	void OutputPointCloudDataIfReady();
};
