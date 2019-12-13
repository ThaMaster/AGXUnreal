// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/Constraint1DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

FConstraint1DOFBarrier::FConstraint1DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint1DOFBarrier::FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint1DOFBarrier::~FConstraint1DOFBarrier()
{
}

void FConstraint1DOFBarrier::SetElectricMotorController(
	const FElectricMotorControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::ElectricMotorController* NativeController = NativeCasted->getElectricMotorController();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::SetFrictionController(
	const FFrictionControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::FrictionController* NativeController = NativeCasted->getFrictionController();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::SetLockController(const FLockControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::LockController* NativeController = NativeCasted->getLock1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::SetRangeController(const FRangeControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::RangeController* NativeController = NativeCasted->getRange1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::SetTargetSpeedController(
	const FTargetSpeedControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::TargetSpeedController* NativeController = NativeCasted->getMotor1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

agx::Constraint1DOF* FConstraint1DOFBarrier::GetNativeCasted() const
{
	if (HasNative())
		return static_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	else
		return nullptr;
}
