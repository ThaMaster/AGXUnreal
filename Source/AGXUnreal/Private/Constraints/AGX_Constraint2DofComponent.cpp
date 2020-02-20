#include "Constraints/AGX_Constraint2DofComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint2DOFBarrier.h"

UAGX_Constraint2DofComponent::UAGX_Constraint2DofComponent()
{
}

UAGX_Constraint2DofComponent::UAGX_Constraint2DofComponent(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational,
	bool bIsSecondaryConstraint2Rotational)
	: UAGX_ConstraintComponent(LockedDofsOrdered)
	, ElectricMotorController1(bIsSecondaryConstraint1Rotational)
	, ElectricMotorController2(bIsSecondaryConstraint2Rotational)
	, FrictionController1(bIsSecondaryConstraint1Rotational)
	, FrictionController2(bIsSecondaryConstraint2Rotational)
	, LockController1(bIsSecondaryConstraint1Rotational)
	, LockController2(bIsSecondaryConstraint2Rotational)
	, RangeController1(bIsSecondaryConstraint1Rotational)
	, RangeController2(bIsSecondaryConstraint2Rotational)
	, TargetSpeedController1(bIsSecondaryConstraint1Rotational)
	, TargetSpeedController2(bIsSecondaryConstraint2Rotational)
{
}

UAGX_Constraint2DofComponent::~UAGX_Constraint2DofComponent()
{
}

void UAGX_Constraint2DofComponent::UpdateNativeProperties()
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

FConstraint2DOFBarrier* UAGX_Constraint2DofComponent::GetNativeBarrierCasted() const
{
	if (HasNative())
		return static_cast<FConstraint2DOFBarrier*>(NativeBarrier.Get());
	else
		return nullptr;
}
