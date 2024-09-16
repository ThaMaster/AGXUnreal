// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies the model of the Lidar. */
UENUM(BlueprintType)
enum EAGX_LidarModel
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
enum EAGX_LidarSurfaceMaterialAssignmentSelection
{
	/**
	 * Assign the Lidar Surface Material to the parent Component. Searches up the Component hierarcy
	 * for the first valid parent.
	 */
	LSM_Parent,

	/**
	 * Assign the Lidar Surface Material to all sibling Components, i.e. all Components that share
	 * the same immediate Parent.
	 */
	LSM_Siblings,

	/**
	 * Assign the Lidar Surface Material to all child Components. Searches recursively.
	 */
	LSM_Children,
};

/** Number of channels setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSChannelCount
{
	CH_32,
	CH_64,
	CH_128
};

/** Channel distribution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSChannelDistribution
{
	CD_Uniform,
	CD_AboveHorizon,
	CD_BelowHorizon
};

/** Horizontal resolution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSHorizontalResolution
{
	HR_512,
	HR_1024,
	HR_2048
};

/** Frequency setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSFrequency
{
	F_10,
	F_20
};

/** Ray angle noise axis setting. */
UENUM(BlueprintType)
enum EAGX_LidarRayAngleDistortionAxis
{
	DA_X,
	DA_Y,
	DA_Z
};
