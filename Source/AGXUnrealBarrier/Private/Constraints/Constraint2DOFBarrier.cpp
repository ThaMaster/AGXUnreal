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
