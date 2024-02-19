// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ROS2Utilities.generated.h"

struct FAGX_LidarScanPoint;
struct FAGX_SensorMsgsImage;
struct FAGX_SensorMsgsPointCloud2;

class AGXUNREAL_API FAGX_ROS2Utilities
{
public:
	static FAGX_SensorMsgsImage Convert(
		const TArray<FColor>& Image, double TimeStamp, const FIntPoint& Resolution, bool Grayscale);

	static FAGX_SensorMsgsImage Convert(
		const TArray<FFloat16Color>& Image, double TimeStamp, const FIntPoint& Resolution,
		bool Grayscale);
};

UCLASS(ClassGroup = "AGX ROS2 Utilities")
class AGXUNREAL_API UAGX_ROS2Utilities : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	/**
	 * Takes an array of Lidar Scan Points and converts it into a ROS2 sensor_msgs::PointCloud2
	 * message.
	 * The Data member consists of position X, Y, Z and Intensity for each point written as double's
	 * in little endian layout, i.e. 32 bytes per point.
	 *
	 * Note that all invalid points, such as points representing scan misses, are ignored.
	 * This means that the sensor_msgs::PointCloud2 message created by this function is always
	 * dense.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set to
	 * the timestamp of the first valid Point in the given array, even if other points have been
	 * generated at later timestamps.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
	static FAGX_SensorMsgsPointCloud2 ConvertXYZ(
		const TArray<FAGX_LidarScanPoint>& Points);

	/**
	 * Takes an array of Lidar Scan Points and converts it into a ROS2 sensor_msgs::PointCloud2
	 * message.
	 * The Data member consists of AngleX [rad] (double), AngleY [rad] (double), time of flight
	 * (TOF) [ps] (uint32) and Intensity (double) for each point in little endian layout, i.e. 28
	 * bytes per point. The speed of light used for the TOF calculation is the speed of light in
	 * vacuum.
	 *
	 * Note that all invalid points, such as points representing scan misses, are ignored.
	 * This means that the sensor_msgs::PointCloud2 message created by this function is always
	 * dense.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set to
	 * the timestamp of the first valid Point in the given array, even if other points have been
	 * generated at later timestamps.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static FAGX_SensorMsgsPointCloud2 ConvertAnglesTOF(
		const TArray<FAGX_LidarScanPoint>& Points);
};
