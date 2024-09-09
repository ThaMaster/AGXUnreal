// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FCustomPatternFetcherBase;
class FLidarOutputBarrier;
class UAGX_LidarModelParameters;

struct FAGX_RealInterval;
struct FDistanceGaussianNoiseRef;
struct FLidarRef;

class AGXUNREALBARRIER_API FLidarBarrier
{
public:
	FLidarBarrier();
	FLidarBarrier(std::unique_ptr<FLidarRef> Native);
	FLidarBarrier(FLidarBarrier&& Other);
	~FLidarBarrier();

	bool HasNative() const;
	void AllocateNative(EAGX_LidarModel Model, const UAGX_LidarModelParameters& Params);
	void AllocateNativeCustomRayPattern(FCustomPatternFetcherBase& PatternFetcher);
	FLidarRef* GetNative();
	const FLidarRef* GetNative() const;
	void ReleaseNative();

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetTransform(const FTransform& Transform);

	void SetRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetRange() const;

	void SetBeamDivergence(double BeamDivergence);
	double GetBeamDivergence() const;

	void SetBeamExitRadius(double BeamExitRadius);
	double GetBeamExitRadius() const;

	void SetEnableRemoveRayMisses(bool bEnable);
	bool GetEnableRemoveRayMisses() const;

	void EnableDistanceGaussianNoise(double Mean, double StdDev, double StdDevSlope);
	void DisableDistanceGaussianNoise();
	bool IsDistanceGaussianNoiseEnabled() const;

	void AddOutput(FLidarOutputBarrier& Output);

private:
	FLidarBarrier(const FLidarBarrier&) = delete;
	void operator=(const FLidarBarrier&) = delete;

private:
	std::unique_ptr<FLidarRef> NativeRef;
	std::unique_ptr<FDistanceGaussianNoiseRef> DistanceNoiseNativeRef;
};
