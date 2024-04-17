// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarResultBarrier.h"

struct FAGX_LidarResultPositionIntensityData;

class AGXUNREALBARRIER_API FLidarResultPositionIntensityBarrier : public FLidarResultBarrier
{
public:

	virtual void AllocateNative() override;

	void GetResult(TArray<FAGX_LidarResultPositionIntensityData>& OutResult) const;
};
