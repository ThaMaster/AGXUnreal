// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FCustomPatternFetcherBase;

struct FLidarRef;

class AGXUNREALBARRIER_API FLidarBarrier
{
public:
	FLidarBarrier();
	FLidarBarrier(std::unique_ptr<FLidarRef> Native);
	FLidarBarrier(FLidarBarrier&& Other);
	~FLidarBarrier();

	bool HasNative() const;
	void AllocateNativeRayPatternHorizontalSweep(
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
	
	void SetBeamExitDiameter(double BeamExitDiameter);
	double GetBeamExitDiameter() const;

	void GetResultTest(UWorld* World, const FTransform& Transform); // Temp test function, do not merge!

private:
	FLidarBarrier(const FLidarBarrier&) = delete;
	void operator=(const FLidarBarrier&) = delete;

private:
	std::unique_ptr<FLidarRef> NativeRef;
};
