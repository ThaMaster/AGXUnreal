// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FElectricMotorControllerBarrier;
struct FFrictionControllerBarrier;
struct FLockControllerBarrier;
struct FRangeControllerBarrier;
struct FScrewControllerBarrier;
struct FTargetSpeedControllerBarrier;

namespace agx
{
	class Constraint2DOF;
}

class AGXUNREALBARRIER_API FConstraint2DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint2DOFBarrier();
	FConstraint2DOFBarrier(FConstraint2DOFBarrier&& Other) = default;
	FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraint2DOFBarrier();

	void SetElectricMotorController(
		const FElectricMotorControllerBarrier& Controller, int32 SecondaryConstraintIndex);

	void GetElectricMotorController(
		FElectricMotorControllerBarrier& Controller, int32 SecondaryConstraintIndex) const;

	void SetFrictionController(
		const FFrictionControllerBarrier& Controller, int32 SecondaryConstraintIndex);

	void GetFrictionController(
		FFrictionControllerBarrier& Controller, int32 SecondaryConstraintIndex) const;

	void SetLockController(
		const FLockControllerBarrier& Controller, int32 SecondaryConstraintIndex);

	void GetLockController(
		FLockControllerBarrier& Controller, int32 SecondaryConstraintIndex) const;

	void SetRangeController(
		const FRangeControllerBarrier& Controller, int32 SecondaryConstraintIndex);

	void GetRangeController(
		FRangeControllerBarrier& Controller, int32 SecondaryConstraintIndex) const;

	void SetTargetSpeedController(
		const FTargetSpeedControllerBarrier& Controller, int32 SecondaryConstraintIndex);

	void GetTargetSpeedController(
		FTargetSpeedControllerBarrier& Controller, int32 SecondaryConstraintIndex) const;

	void SetScrewController(const FScrewControllerBarrier& Controller);

	void GetScrewController(FScrewControllerBarrier& Controller) const;

private:
	FConstraint2DOFBarrier(const FConstraint2DOFBarrier&) = delete;
	void operator=(const FConstraint2DOFBarrier&) = delete;

	agx::Constraint2DOF* GetNativeCasted() const;
};
