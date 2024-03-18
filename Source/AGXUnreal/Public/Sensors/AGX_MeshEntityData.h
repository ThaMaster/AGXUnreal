// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/RtEntityBarrier.h"
#include "Sensors/RtMeshBarrier.h"


struct FAGX_EntityData
{
	FRtEntityBarrier Entity;
	FTransform Transform;
	size_t RefCount {1};

	void SetTransform(const FTransform& InTransform)
	{
		Transform = InTransform;

		if (Entity.HasNative())
			Entity.SetTransform(InTransform);
	}
};

struct FAGX_MeshEntityData
{
	FRtMeshBarrier Mesh;
	FAGX_EntityData EntityData;
};

struct FAGX_InstancedMeshEntityData
{
	FRtMeshBarrier Mesh;
	TMap<int32, FAGX_MeshEntityData> EntitiesData;
};
