// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// AGXUnreal includes.
#include "Constraints/ConstraintBarrier.h"

// Unreal Engine includes.
#include "Templates/UniquePtr.h"

class FElectricMotorControllerBarrier;
class FFrictionControllerBarrier;
class FLockControllerBarrier;
class FRangeControllerBarrier;
class FTargetSpeedControllerBarrier;

namespace agx
{
	class Constraint1DOF;
}

class AGXUNREALBARRIER_API FConstraint1DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint1DOFBarrier();
	FConstraint1DOFBarrier(FConstraint1DOFBarrier&& Other) = default;
	FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraint1DOFBarrier();

	TUniquePtr<FElectricMotorControllerBarrier> GetElectricMotorController();
	TUniquePtr<FFrictionControllerBarrier> GetFrictionController();
	TUniquePtr<FLockControllerBarrier> GetLockController();
	TUniquePtr<FRangeControllerBarrier> GetRangeController();
	TUniquePtr<FTargetSpeedControllerBarrier> GetTargetSpeedController();

	TUniquePtr<const FElectricMotorControllerBarrier> GetElectricMotorController() const;
	TUniquePtr<const FFrictionControllerBarrier> GetFrictionController() const;
	TUniquePtr<const FLockControllerBarrier> GetLockController() const;
	TUniquePtr<const FRangeControllerBarrier> GetRangeController() const;
	TUniquePtr<const FTargetSpeedControllerBarrier> GetTargetSpeedController() const;

private:
	FConstraint1DOFBarrier(const FConstraint1DOFBarrier&) = delete;
	void operator=(const FConstraint1DOFBarrier&) = delete;
};
