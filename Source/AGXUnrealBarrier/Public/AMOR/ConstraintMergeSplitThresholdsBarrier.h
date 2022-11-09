// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class FConstraintBarrier;

class AGXUNREALBARRIER_API FConstraintMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FConstraintMergeSplitThresholdsBarrier();
	FConstraintMergeSplitThresholdsBarrier(FConstraintMergeSplitThresholdsBarrier&& Other) = default;
	FConstraintMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native);
	~FConstraintMergeSplitThresholdsBarrier();

	void AllocateNative(bool bInIsRotational);

	void SetMaxDesiredForceRangeDiff(double InMaxDesiredForceRangeDiff);
	double GetMaxDesiredForceRangeDiff() const;

	void SetMaxDesiredLockAngleDiff(double InMaxDesiredLockAngleDiff);
	double GetMaxDesiredLockAngleDiff() const;

	void SetMaxDesiredRangeAngleDiff(double InMaxDesiredRangeAngleDiff);
	double GetMaxDesiredRangeAngleDiff() const;

	void SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff);
	double GetMaxDesiredSpeedDiff() const;

	void SetMaxRelativeSpeed(double InMaxRelativeSpeed);
	double GetMaxRelativeSpeed() const;

	void SetIsRotational(bool InIsRotational);
	bool GetIsRotational() const;

	static FConstraintMergeSplitThresholdsBarrier CreateFrom(const FConstraintBarrier& Barrier);

private:
	FConstraintMergeSplitThresholdsBarrier(const FConstraintMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FConstraintMergeSplitThresholdsBarrier&) = delete;

	bool bIsRotational = true;
};