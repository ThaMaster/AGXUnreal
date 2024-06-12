// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

//  Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

struct FElementaryConstraintRef;

class AGXUNREALBARRIER_API FElementaryConstraintBarrier
{
public:
	FElementaryConstraintBarrier();
	FElementaryConstraintBarrier(std::unique_ptr<FElementaryConstraintRef> InNative);
	virtual ~FElementaryConstraintBarrier();

	/**
	 * Make this Barrier point to the same native AGX Dynamics object as Other. The Ref object
	 * will not be shared, only the AGX Dynamics object.
	 */
	FElementaryConstraintBarrier& operator=(const FElementaryConstraintBarrier& Other) = delete;

	bool HasNative() const;
	//void SetNative(FElementaryConstraintRef* InNative);
	FElementaryConstraintRef* GetNative();
	const FElementaryConstraintRef* GetNative() const;

	void SetEnable(bool bEnabled);
	bool GetEnable() const;

	bool IsActive() const;

	void SetCompliance(double InCompliance, int32 InRow);
	void SetCompliance(double InCompliance);
	double GetCompliance(int32 InRow = 0) const;

	void SetElasticity(double Elasticity, int Row);
	void SetElasticity(double elasticity);
	double GetElasticity(int32 row = 0) const;

	void SetSpookDamping(double InDamping, int32 InRow);
	void SetSpookDamping(double InDamping);
	double GetSpookDamping(int32 InRow = 0) const;

	void SetForceRange(double InMin, double InMax, int32 InRow = 0);
	void SetForceRange(FAGX_RealInterval InForceRange, int32 InRow = 0);
	void SetForceRange(FDoubleInterval InForceRange, int32 InRow = 0);
	void SetForceRangeMin(double InMin, int32 InRow = 0);
	void SetForceRangeMax(double InMax, int32 InRow = 0);
	FDoubleInterval GetForceRange(int32 InRow = 0) const;
	double GetForceRangeMin(int32 InRow = 0) const;
	double GetForceRangeMax(int32 InRow = 0) const;

	double GetForce(int32 InRow = 0) const;

	int32 GetNumRows() const;

	FString GetName() const;

protected:
	std::unique_ptr<FElementaryConstraintRef> NativeRef;
};
