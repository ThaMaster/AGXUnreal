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

void FConstraint1DOFBarrier::SetRangeController(const FRangeControllerBarrier &RangeController, UWorld* World)
{
	check(HasNative());

	agx::RangeController* NativeController = NATIVE_CASTED->getRange1D();

	RangeController.ToNative(NativeController, World);
}
