// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/RtEntityBarrier.h"
#include "Sensors/RtMeshBarrier.h"


struct FAGX_MeshEntityData
{
	FRtMeshBarrier Mesh;
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
