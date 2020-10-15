#include "Constraints/AGX_Constraint1DofComponent.h"

#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Constraint1DOFBarrier.h"

UAGX_Constraint1DofComponent::UAGX_Constraint1DofComponent()
{
}

UAGX_Constraint1DofComponent::UAGX_Constraint1DofComponent(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
	bool bInIsLockControllerEditable)
	: UAGX_ConstraintComponent(LockedDofsOrdered)
	, ElectricMotorController(bIsSecondaryConstraintRotational)
	, FrictionController(bIsSecondaryConstraintRotational)
	, LockController(bIsSecondaryConstraintRotational)
	, RangeController(bIsSecondaryConstraintRotational)
	, TargetSpeedController(bIsSecondaryConstraintRotational)
	, bIsLockControllerEditable(bInIsLockControllerEditable)
{
}

UAGX_Constraint1DofComponent::~UAGX_Constraint1DofComponent()
{
}

namespace
{
	FConstraint1DOFBarrier* Get1DOFBarrier(UAGX_Constraint1DofComponent& Constraint)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FConstraint1DOFBarrier*>(Constraint.GetNative());
	}

	const FConstraint1DOFBarrier* Get1DOFBarrier(const UAGX_Constraint1DofComponent& Constraint)
	{
		return static_cast<const FConstraint1DOFBarrier*>(Constraint.GetNative());
	}
}

float UAGX_Constraint1DofComponent::GetAngle() const
{
	return Get1DOFBarrier(*this)->GetAngle();
}

void UAGX_Constraint1DofComponent::CreateNativeImpl()
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

void UAGX_Constraint1DofComponent::UpdateNativeProperties()
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
