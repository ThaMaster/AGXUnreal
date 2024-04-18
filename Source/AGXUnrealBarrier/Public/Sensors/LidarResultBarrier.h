// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FLidarResultRef;


class AGXUNREALBARRIER_API FLidarResultBarrier
{
public:
	FLidarResultBarrier();
	FLidarResultBarrier(std::unique_ptr<FLidarResultRef> Native);
	FLidarResultBarrier(FLidarResultBarrier&& Other);
	virtual ~FLidarResultBarrier();

	virtual void AllocateNative() = 0;

	bool HasNative() const;
	FLidarResultRef* GetNative();
	const FLidarResultRef* GetNative() const;
	void ReleaseNative();

	bool EnableDistanceGaussianNoise(double Mean, double StdDev, double StdDevSlope);
	bool DisableDistanceGaussianNoise();
	bool IsDistanceGaussianNoiseEnabled() const;

	bool EnableRemovePointsMisses(bool bEnable);
	bool IsRemovePointsMissesEnabled() const;

private:
	FLidarResultBarrier(const FLidarResultBarrier&) = delete;
	void operator=(const FLidarResultBarrier&) = delete;

protected:
	std::unique_ptr<FLidarResultRef> NativeRef;
};
