//

#include "Constraints/AGX_Constraint1DOF.h"

#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Constraint1DOFBarrier.h"

AAGX_Constraint1DOF::AAGX_Constraint1DOF()
{
}

AAGX_Constraint1DOF::AAGX_Constraint1DOF(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
	bool bInIsLockControllerEditable)
	: AAGX_Constraint(LockedDofsOrdered)
	, ElectricMotorController(bIsSecondaryConstraintRotational)
	, FrictionController(bIsSecondaryConstraintRotational)
	, LockController(bIsSecondaryConstraintRotational)
	, RangeController(bIsSecondaryConstraintRotational)
	, TargetSpeedController(bIsSecondaryConstraintRotational)
	, bIsLockControllerEditable(bInIsLockControllerEditable)
{
}

AAGX_Constraint1DOF::~AAGX_Constraint1DOF()
{
}

namespace
{
	FConstraint1DOFBarrier* Get1DOFBarrier(AAGX_Constraint1DOF& Constraint)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FConstraint1DOFBarrier*>(Constraint.GetNative());
	}
}

void AAGX_Constraint1DOF::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	FConstraint1DOFBarrier* Barrier = Get1DOFBarrier(*this);
	ElectricMotorController.InitializeBarrier(Barrier->GetElectricMotorController());
	FrictionController.InitializeBarrier(Barrier->GetFrictionController());
	LockController.InitializeBarrier(Barrier->GetLockController());
	RangeController.InitializeBarrier(Barrier->GetRangeController());
	TargetSpeedController.InitializeBarrier(Barrier->GetTargetSpeedController());
}

void AAGX_Constraint1DOF::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Constraint '%s' is trying to update native properties while not having a "
				 "native handle."), *GetName());
		return;
	}

	Super::UpdateNativeProperties();

	ElectricMotorController.UpdateNativeProperties();
	FrictionController.UpdateNativeProperties();
	LockController.UpdateNativeProperties();
	RangeController.UpdateNativeProperties();
	TargetSpeedController.UpdateNativeProperties();
}
