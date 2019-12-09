// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_Constraint1DOF.h"

#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint1DOFBarrier.h"

AAGX_Constraint1DOF::AAGX_Constraint1DOF()
{
}

AAGX_Constraint1DOF::AAGX_Constraint1DOF(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational, bool bIsLockControllerEditable_)
	: AAGX_Constraint(LockedDofsOrdered)
	, RangeController(bIsSecondaryConstraintRotational)
	, bIsLockControllerEditable(bIsLockControllerEditable_)
{
}

AAGX_Constraint1DOF::~AAGX_Constraint1DOF()
{
}

void AAGX_Constraint1DOF::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();

	// TODO: To avoid constructing the controller barrier structs on the stack
	// everytime native data needs to be synced, consider letting the Unreal Controller
	// instead have a lifetime-bound dynamically created instance of the controller barrier.

	if (FConstraint1DOFBarrier* NativeBarrierCasted = GetNativeBarrierCasted())
	{
		// Electric Motor Controller
		{
			FElectricMotorControllerBarrier ControllerBarrier;
			ElectricMotorController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetElectricMotorController(ControllerBarrier);
		}

		// Friction Controller
		{
			FFrictionControllerBarrier ControllerBarrier;
			FrictionController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetFrictionController(ControllerBarrier);
		}

		// Lock Controller
		if (bIsLockControllerEditable)
		{
			FLockControllerBarrier ControllerBarrier;
			LockController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetLockController(ControllerBarrier);
		}

		// Range Controller
		{
			FRangeControllerBarrier ControllerBarrier;
			RangeController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetRangeController(ControllerBarrier);
		}

		// Target Speed Controller
		{
			FTargetSpeedControllerBarrier ControllerBarrier;
			TargetSpeedController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetTargetSpeedController(ControllerBarrier);
		}
	}
}

FConstraint1DOFBarrier* AAGX_Constraint1DOF::GetNativeBarrierCasted() const
{
	if (HasNative())
		return static_cast<FConstraint1DOFBarrier*>(NativeBarrier.Get());
	else
		return nullptr;
}
