// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FRangeControllerBarrier;
struct FTargetSpeedControllerBarrier;

class AGXUNREALBARRIER_API FConstraint2DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint2DOFBarrier();
	virtual ~FConstraint2DOFBarrier();

	void SetRangeController(const FRangeControllerBarrier &Controller, int32 SecondaryConstraintIndex, UWorld* World);

	void SetTargetSpeedController(const FTargetSpeedControllerBarrier &Controller, int32 SecondaryConstraintIndex, UWorld* World);
	
private:
	FConstraint2DOFBarrier(const FConstraint2DOFBarrier&) = delete;
	void operator=(const FConstraint2DOFBarrier&) = delete;
};
