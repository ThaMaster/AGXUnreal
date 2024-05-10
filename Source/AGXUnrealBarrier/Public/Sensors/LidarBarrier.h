// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FCustomPatternFetcherBase;
class FLidarOutputBarrier;

struct FLidarRef;

class AGXUNREALBARRIER_API FLidarBarrier
{
public:
	FLidarBarrier();
	FLidarBarrier(std::unique_ptr<FLidarRef> Native);
	FLidarBarrier(FLidarBarrier&& Other);
	~FLidarBarrier();

	bool HasNative() const;
	void AllocateNativeLidarRayPatternHorizontalSweep(
		const FVector2D& FOV, const FVector2D& Resolution, double Frequency);
	void AllocateNativeRayPatternCustom(FCustomPatternFetcherBase* PatternFetcher);
	FLidarRef* GetNative();
	const FLidarRef* GetNative() const;
	void ReleaseNative();

	void SetTransform(const FTransform& Transform);

	void SetRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetRange() const;

	void SetBeamDivergence(double BeamDivergence);
	double GetBeamDivergence() const;
	
	void SetBeamExitRadius(double BeamExitRadius);
	double GetBeamExitRadius() const;

	bool EnableDistanceGaussianNoise(double Mean, double StdDev, double StdDevSlope);
	bool DisableDistanceGaussianNoise();
	bool IsDistanceGaussianNoiseEnabled() const;

	void SetEnableRemoveRayMisses(bool bEnable);
	bool GetEnableRemoveRayMisses() const;

	void AddResult(FLidarOutputBarrier& Result);

private:
	FLidarBarrier(const FLidarBarrier&) = delete;
	void operator=(const FLidarBarrier&) = delete;

private:
	std::unique_ptr<FLidarRef> NativeRef;
};
