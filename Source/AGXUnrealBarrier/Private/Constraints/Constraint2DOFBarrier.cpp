// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/Constraint2DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"


FConstraint2DOFBarrier::FConstraint2DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint2DOFBarrier::FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint2DOFBarrier::~FConstraint2DOFBarrier()
{
}


void FConstraint2DOFBarrier::SetElectricMotorController(
	const FElectricMotorControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::ElectricMotorController* NativeController =
			NativeCasted->getElectricMotorController((agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}


void FConstraint2DOFBarrier::SetFrictionController(
	const FFrictionControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::FrictionController* NativeController =
			NativeCasted->getFrictionController((agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}


void FConstraint2DOFBarrier::SetLockController(
	const FLockControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::LockController* NativeController =
			NativeCasted->getLock1D((agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}


void FConstraint2DOFBarrier::SetRangeController(
	const FRangeControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::RangeController* NativeController =
			NativeCasted->getRange1D((agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}


void FConstraint2DOFBarrier::SetTargetSpeedController(
	const FTargetSpeedControllerBarrier &ControllerBarrier,
	int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::TargetSpeedController* NativeController =
			NativeCasted->getMotor1D((agx::Constraint2DOF::DOF)SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}


void FConstraint2DOFBarrier::SetScrewController(
	const FScrewControllerBarrier &ControllerBarrier)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::ScrewController* NativeController = NativeCasted->getScrew1D();

		ControllerBarrier.ToNative(NativeController);
	}
}


agx::Constraint2DOF* FConstraint2DOFBarrier::GetNativeCasted() const
{
	if (HasNative())
		return static_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	else
		return nullptr;
}
