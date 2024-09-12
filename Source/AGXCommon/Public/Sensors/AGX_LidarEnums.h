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
enum EAGX_OusterOSChannelCount
{
	Ch_32,
	Ch_64,
	Ch_128
};

/** Channel distribution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSChannelDistribution
{
	Uniform,
	AboveHorizon,
	BelowHorizon
};

/** Horizontal resolution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSHorizontalResolution
{
	Hr_512,
	Hr_1024,
	Hr_2048
};

/** Frequency setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum EAGX_OusterOSFrequency
{
	F_10,
	F_20
};
