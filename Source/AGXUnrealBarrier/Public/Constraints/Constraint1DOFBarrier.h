// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FElectricMotorControllerBarrier;
struct FFrictionControllerBarrier;
struct FLockControllerBarrier;
struct FRangeControllerBarrier;
struct FTargetSpeedControllerBarrier;

namespace agx {
	class Constraint1DOF;
}

class AGXUNREALBARRIER_API FConstraint1DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint1DOFBarrier();
	FConstraint1DOFBarrier(FConstraint1DOFBarrier&& Other);
	FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraint1DOFBarrier();

	void SetElectricMotorController(const FElectricMotorControllerBarrier &Controller);

	void SetFrictionController(const FFrictionControllerBarrier &Controller);

	void SetLockController(const FLockControllerBarrier &Controller);

	void SetRangeController(const FRangeControllerBarrier &Controller);

	void SetTargetSpeedController(const FTargetSpeedControllerBarrier &Controller);

private:
	FConstraint1DOFBarrier(const FConstraint1DOFBarrier&) = delete;
	void operator=(const FConstraint1DOFBarrier&) = delete;

	agx::Constraint1DOF* GetNativeCasted() const;
};
