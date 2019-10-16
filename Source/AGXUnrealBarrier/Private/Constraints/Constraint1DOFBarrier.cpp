// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/Constraint1DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"


#define NATIVE_CASTED static_cast<agx::Constraint1DOF*>(NativeRef->Native.get())


FConstraint1DOFBarrier::FConstraint1DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint1DOFBarrier::~FConstraint1DOFBarrier()
{
}

void FConstraint1DOFBarrier::SetElectricMotorController(const FElectricMotorControllerBarrier &ControllerBarrier, UWorld* World)
{
	check(HasNative());

	agx::ElectricMotorController* NativeController = NATIVE_CASTED->getElectricMotorController();

	ControllerBarrier.ToNative(NativeController, World);
}

void FConstraint1DOFBarrier::SetFrictionController(const FFrictionControllerBarrier &ControllerBarrier, UWorld* World)
{
	check(HasNative());

	agx::FrictionController* NativeController = NATIVE_CASTED->getFrictionController();

	ControllerBarrier.ToNative(NativeController, World);
}

void FConstraint1DOFBarrier::SetRangeController(const FRangeControllerBarrier &ControllerBarrier, UWorld* World)
{
	check(HasNative());

	agx::RangeController* NativeController = NATIVE_CASTED->getRange1D();

	ControllerBarrier.ToNative(NativeController, World);
}

void FConstraint1DOFBarrier::SetTargetSpeedController(const FTargetSpeedControllerBarrier &ControllerBarrier, UWorld* World)
{
	check(HasNative());

	agx::TargetSpeedController* NativeController = NATIVE_CASTED->getMotor1D();

	ControllerBarrier.ToNative(NativeController, World);
}