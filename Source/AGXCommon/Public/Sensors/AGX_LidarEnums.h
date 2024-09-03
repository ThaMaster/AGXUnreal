// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies the model of the Lidar. */
UENUM()
enum class EAGX_LidarModel
{
	/** Generic Lidar with a Horizontal Sweep ray pattern. */
	GenericHorizontalSweep,

	/** Lidar uses a custom ray pattern where the user provides the ray pattern rays. */
	Custom
};

/** Specifies which Components the Lidar Surface Material Component applies the Lidar Surface
 * Material to. */
UENUM()
enum class EAGX_LidarSurfaceMaterialAssignmentSelection
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
