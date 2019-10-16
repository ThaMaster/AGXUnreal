// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FRangeControllerBarrier;

class AGXUNREALBARRIER_API FConstraint1DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint1DOFBarrier();
	virtual ~FConstraint1DOFBarrier();

	void SetRangeController(const FRangeControllerBarrier &RangeController, UWorld* World);
	
private:
	FConstraint1DOFBarrier(const FConstraint1DOFBarrier&) = delete;
	void operator=(const FConstraint1DOFBarrier&) = delete;
};
