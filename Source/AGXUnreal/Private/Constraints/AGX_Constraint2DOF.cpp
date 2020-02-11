#include "Constraints/AGX_Constraint2DOF.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint2DOFBarrier.h"

AAGX_Constraint2DOF::AAGX_Constraint2DOF()
{
}

AAGX_Constraint2DOF::AAGX_Constraint2DOF(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational,
	bool bIsSecondaryConstraint2Rotational)
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
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Constraint '%s' is trying to update native properties while not having a "
				 "native handle."), *GetName());
		return;
	}

	Super::UpdateNativeProperties();

	/// \todo Perhaps add a function that returns a list of all the controllers.
	ElectricMotorController1.UpdateNativeProperties();
	ElectricMotorController2.UpdateNativeProperties();
	FrictionController1.UpdateNativeProperties();
	FrictionController2.UpdateNativeProperties();
	LockController1.UpdateNativeProperties();
	LockController2.UpdateNativeProperties();
	RangeController1.UpdateNativeProperties();
	RangeController2.UpdateNativeProperties();
	TargetSpeedController1.UpdateNativeProperties();
	TargetSpeedController2.UpdateNativeProperties();
}

FConstraint2DOFBarrier* AAGX_Constraint2DOF::GetNativeBarrierCasted() const
{
	if (HasNative())
		return static_cast<FConstraint2DOFBarrier*>(NativeBarrier.Get());
	else
		return nullptr;
}
