// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/ElementaryConstraintBarrier.h"

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier : public FElementaryConstraintBarrier
{
public: // Type declarations.
	using Super = FElementaryConstraintBarrier;

public: // Special member functions.
	FTwistRangeControllerBarrier();
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);
	FTwistRangeControllerBarrier(std::unique_ptr<FElementaryConstraintRef> InNative);
	virtual ~FTwistRangeControllerBarrier();
	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

public: // Native management.
	virtual void SetNative(FElementaryConstraintRef* InNative) override;

public: // AGX Dynamics accessors.
	void SetRange(FDoubleInterval InRange);
	void SetRange(FAGX_RealInterval InRange);
	void SetRange(double InMin, double InMax);
	void SetRangeMin(double InMin);
	void SetRangeMax(double InMax);

	FDoubleInterval GetRange() const;
	double GetRangeMin() const;
	double GetRangeMax() const;

private:
	bool CheckValidNative();
};
