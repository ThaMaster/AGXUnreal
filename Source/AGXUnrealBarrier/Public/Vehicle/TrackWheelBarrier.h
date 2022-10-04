// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"

// Standard library includes.
#include <memory>

struct FTrackWheelRef;

/*
 * This is currently only used by the Import pipeline.
 * In the future, it would be possible to let the FAGX_TrackWheel UStruct own an instance of this
 * class and be responsible to keep it in sync, and expose relevant functions to the user in the
 * Editor.
 */
class AGXUNREALBARRIER_API FTrackWheelBarrier
{
public:
	FTrackWheelBarrier();
	FTrackWheelBarrier(std::unique_ptr<FTrackWheelRef> Native);
	FTrackWheelBarrier(FTrackWheelBarrier&& Other);
	~FTrackWheelBarrier();

	bool HasNative() const;

	FRigidBodyBarrier GetRigidBody() const;

	double GetRadius() const;

	uint8 GetModel() const;

	bool GetSplitSegments() const;

	bool GetMoveNodesToRotationPlane() const;

	bool GetMoveNodesToWheel() const;


private:
	std::unique_ptr<FTrackWheelRef> NativeRef;
};