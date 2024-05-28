// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Standard library includes.
#include <memory>

struct FElementaryConstraintRef;

class AGXUNREALBARRIER_API FElementaryConstraintBarrier
{
public:
	FElementaryConstraintBarrier(std::unique_ptr<FElementaryConstraintRef> InNative);
	virtual ~FElementaryConstraintBarrier();

	bool HasNative() const;
	FElementaryConstraintRef* GetNative();
	const FElementaryConstraintRef* GetNative() const;

	void SetEnable(bool bEnabled);
	bool GetEnable() const;

	void SetCompliance(double InCompliance, int32 InRow);
	void SetCompliance(double InCompliance);
	double GetCompliance(int32 InRow = 0) const;

	void SetSpookDamping(double InDamping, int32 InRow = 0);
	void SetSpookDamping(double InDamping);
	double GetDamping(int32 InRow = 0) const;

	void SetForceRange(double InMin, double InMax, int32 InRow = 0);
	void SetForceRange(FAGX_RealInterval InForceRange, int32 InRow = 0);
	FAGX_RealInterval GetForceRange(int32 InRow = 0) const;
	double GetForceRangeMin(int32 InRow = 0) const;
	double GetForceRangeMax(int32 InRow = 0) const;

	double GetForce(int32 InRow = 0) const;

	int32 GetNumRows() const;

protected:
	FElementaryConstraintBarrier();

	std::unique_ptr<FElementaryConstraintRef> NativeRef;
};
