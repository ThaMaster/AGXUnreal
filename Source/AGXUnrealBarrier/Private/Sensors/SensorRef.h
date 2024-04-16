// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Environment.h>
#include <agxSensor/Lidar.h>
#include <agxSensor/RaytraceHandles.h>
#include "EndAGXIncludes.h"

struct FSensorEnvironmentRef
{
	agxSensor::EnvironmentRef Native;
	FSensorEnvironmentRef() = default;
	FSensorEnvironmentRef(agxSensor::Environment* InNative)
		: Native(InNative)
	{
	}
};

struct FRtMeshRef
{
	agxSensor::RtMeshRef Native;
	FRtMeshRef() = default;
	FRtMeshRef(agxSensor::RtMeshRef InNative)
		: Native(InNative)
	{
	}
};

struct FRtMeshEntity
{
	agxSensor::RtMeshEntity Native;
};

struct FLidarRef
{
	agxSensor::LidarRef Native;
	FLidarRef() = default;
	FLidarRef(agxSensor::Lidar* InNative)
		: Native(InNative)
	{
	}
};
