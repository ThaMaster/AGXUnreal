// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies the model of the Lidar. */
UENUM(BlueprintType)
enum class EAGX_LidarModel : uint8
{
	/** Lidar uses a custom ray pattern where the user provides the ray pattern rays. */
	CustomRayPattern = 0,

	/** Generic Lidar with a Horizontal Sweep ray pattern. */
	GenericHorizontalSweep = 5,

	/** Ouster OS0 */
	OusterOS0 = 10,

	/** Ouster OS1 */
	OusterOS1 = 15,

	/** Ouster OS2 */
	OusterOS2 = 20,
};

/** Specifies which Components the Lidar Surface Material Component applies the Lidar Surface
 * Material to. */
UENUM(BlueprintType)
enum class EAGX_LidarSurfaceMaterialAssignmentSelection : uint8
{
	/**
	 * Assign the Lidar Surface Material to the parent Component. Searches up the Component hierarcy
	 * for the first valid parent.
	 */
	Parent,

	/**
	 * Assign the Lidar Surface Material to all sibling Components, i.e. all Components that share
	 * the same immediate Parent.
	 */
	Siblings,

	/**
	 * Assign the Lidar Surface Material to all child Components. Searches recursively.
	 */
	Children,
};

/** Number of channels setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSChannelCount : uint8
{
	CH_32,
	CH_64,
	CH_128
};

/** Channel distribution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSChannelDistribution : uint8
{
	Uniform,
	AboveHorizon,
	BelowHorizon
};

/** Horizontal resolution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSHorizontalResolution : uint8
{
	HR_512,
	HR_1024,
	HR_2048
};

/** Frequency setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSFrequency : uint8
{
	F_10,
	F_20
};

/** Ray angle noise axis setting. */
UENUM(BlueprintType)
enum class EAGX_LidarRayAngleDistortionAxis : uint8
{
	AxisX,
	AxisY,
	AxisZ
};
