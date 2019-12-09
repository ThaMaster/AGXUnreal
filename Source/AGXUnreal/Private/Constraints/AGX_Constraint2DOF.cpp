// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_Constraint2DOF.h"

#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint2DOFBarrier.h"

AAGX_Constraint2DOF::AAGX_Constraint2DOF()
{
}

AAGX_Constraint2DOF::AAGX_Constraint2DOF(const TArray<EDofFlag>& LockedDofsOrdered,
	bool bIsSecondaryConstraint1Rotational, bool bIsSecondaryConstraint2Rotational)
	: AAGX_Constraint(LockedDofsOrdered)
	, RangeController1(bIsSecondaryConstraint1Rotational)
	, RangeController2(bIsSecondaryConstraint2Rotational)
{
}

AAGX_Constraint2DOF::~AAGX_Constraint2DOF()
{
}

void AAGX_Constraint2DOF::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();

	// TODO: To avoid constructing the controller barrier structs on the stack
	// everytime native data needs to be synced, consider letting the Unreal Controller
	// instead have a lifetime-bound dynamically created instance of the controller barrier.

	if (FConstraint2DOFBarrier* NativeBarrierCasted = GetNativeBarrierCasted())
	{
		// Electric Motor Controller 1
		{
			FElectricMotorControllerBarrier ControllerBarrier;
			ElectricMotorController1.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetElectricMotorController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 0);
		}

		// Electric Motor Controller 2
		{
			FElectricMotorControllerBarrier ControllerBarrier;
			ElectricMotorController2.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetElectricMotorController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 1);
		}

		// Friction Controller 1
		{
			FFrictionControllerBarrier ControllerBarrier;
			FrictionController1.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetFrictionController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 0);
		}

		// Friction Controller 2
		{
			FFrictionControllerBarrier ControllerBarrier;
			FrictionController2.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetFrictionController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 1);
		}

		// Lock Controller 1
		{
			FLockControllerBarrier ControllerBarrier;
			LockController1.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetLockController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 0);
		}

		// Lock Controller 2
		{
			FLockControllerBarrier ControllerBarrier;
			LockController2.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetLockController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 1);
		}

		// Range Controller 1
		{
			FRangeControllerBarrier ControllerBarrier;
			RangeController1.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetRangeController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 0);
		}

		// Range Controller 2
		{
			FRangeControllerBarrier ControllerBarrier;
			RangeController2.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetRangeController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 1);
		}

		// Target Speed Controller 1
		{
			FTargetSpeedControllerBarrier ControllerBarrier;
			TargetSpeedController1.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetTargetSpeedController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 0);
		}

		// Target Speed Controller 2
		{
			FTargetSpeedControllerBarrier ControllerBarrier;
			TargetSpeedController2.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetTargetSpeedController(ControllerBarrier, /*bSecondaryConstraintIndex*/ 1);
		}

		// Screw Controller
		{
			FScrewControllerBarrier ControllerBarrier;
			ScrewController.ToBarrier(&ControllerBarrier);
			NativeBarrierCasted->SetScrewController(ControllerBarrier);
		}
	}
}

FConstraint2DOFBarrier* AAGX_Constraint2DOF::GetNativeBarrierCasted() const
{
	if (HasNative())
		return static_cast<FConstraint2DOFBarrier*>(NativeBarrier.Get());
	else
		return nullptr;
}
