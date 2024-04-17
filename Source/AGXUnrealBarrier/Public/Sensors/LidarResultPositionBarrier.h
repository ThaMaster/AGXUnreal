// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarResultTypes.h"
#include "Sensors/LidarResultBarrier.h"


class AGXUNREALBARRIER_API FLidarResultPositionBarrier : public FLidarResultBarrier
{
public:

	virtual void AllocateNative() override;

	void GetResult(TArray<FAGX_LidarResultPositionData>& OutResult) const;
};
