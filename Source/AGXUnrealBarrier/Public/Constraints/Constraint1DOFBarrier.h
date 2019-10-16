// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FElectricMotorControllerBarrier;
struct FFrictionControllerBarrier;
struct FRangeControllerBarrier;
struct FTargetSpeedControllerBarrier;

class AGXUNREALBARRIER_API FConstraint1DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint1DOFBarrier();
	virtual ~FConstraint1DOFBarrier();

	void SetElectricMotorController(const FElectricMotorControllerBarrier &Controller, UWorld* World);

	void SetFrictionController(const FFrictionControllerBarrier &Controller, UWorld* World);
	
	void SetRangeController(const FRangeControllerBarrier &Controller, UWorld* World);

	void SetTargetSpeedController(const FTargetSpeedControllerBarrier &Controller, UWorld* World);
	
private:
	FConstraint1DOFBarrier(const FConstraint1DOFBarrier&) = delete;
	void operator=(const FConstraint1DOFBarrier&) = delete;
};
