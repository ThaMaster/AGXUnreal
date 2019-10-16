// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/Constraint2DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"


#define NATIVE_CASTED static_cast<agx::Constraint2DOF*>(NativeRef->Native.get())


FConstraint2DOFBarrier::FConstraint2DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint2DOFBarrier::~FConstraint2DOFBarrier()
{
}


void FConstraint2DOFBarrier::SetElectricMotorController(
	const FElectricMotorControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex,
	UWorld* World)
{
	check(HasNative());

	agx::ElectricMotorController* NativeController = NATIVE_CASTED->getElectricMotorController(
		(agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

	ControllerBarrier.ToNative(NativeController, World);
}


void FConstraint2DOFBarrier::SetFrictionController(
	const FFrictionControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex,
	UWorld* World)
{
	check(HasNative());

	agx::FrictionController* NativeController = NATIVE_CASTED->getFrictionController(
		(agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

	ControllerBarrier.ToNative(NativeController, World);
}


void FConstraint2DOFBarrier::SetLockController(
	const FLockControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex,
	UWorld* World)
{
	check(HasNative());

	agx::LockController* NativeController = NATIVE_CASTED->getLock1D(
		(agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

	ControllerBarrier.ToNative(NativeController, World);
}


void FConstraint2DOFBarrier::SetRangeController(
	const FRangeControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex,
	UWorld* World)
{
	check(HasNative());

	agx::RangeController* NativeController = NATIVE_CASTED->getRange1D(
		(agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

	ControllerBarrier.ToNative(NativeController, World);
}


void FConstraint2DOFBarrier::SetTargetSpeedController(
	const FTargetSpeedControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex,
	UWorld* World)
{
	check(HasNative());

	agx::TargetSpeedController* NativeController = NATIVE_CASTED->getMotor1D(
		(agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

	ControllerBarrier.ToNative(NativeController, World);
}


void FConstraint2DOFBarrier::SetScrewController(
	const FScrewControllerBarrier &ControllerBarrier,
	UWorld* World)
{
	check(HasNative());

	agx::ScrewController* NativeController = NATIVE_CASTED->getScrew1D();

	ControllerBarrier.ToNative(NativeController, World);
}