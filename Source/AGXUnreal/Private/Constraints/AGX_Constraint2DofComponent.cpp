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
	, ScrewController()
{
}

UAGX_Constraint2DofComponent::~UAGX_Constraint2DofComponent()
{
}

namespace
{
	FConstraint2DOFBarrier* Get2DofBarrier(UAGX_Constraint2DofComponent& Constraint)
	{
		return static_cast<FConstraint2DOFBarrier*>(Constraint.GetNative());
	}
}

void UAGX_Constraint2DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	// Is there a less tedious, and error prone, way to write this?
	EAGX_Constraint2DOFFreeDOF FIRST = EAGX_Constraint2DOFFreeDOF::FIRST;
	EAGX_Constraint2DOFFreeDOF SECOND = EAGX_Constraint2DOFFreeDOF::SECOND;
	FConstraint2DOFBarrier* Barrier = Get2DofBarrier(*this);
	ElectricMotorController1.InitializeBarrier(Barrier->GetElectricMotorController(FIRST));
	ElectricMotorController2.InitializeBarrier(Barrier->GetElectricMotorController(SECOND));
	FrictionController1.InitializeBarrier(Barrier->GetFrictionController(FIRST));
	FrictionController2.InitializeBarrier(Barrier->GetFrictionController(SECOND));
	LockController1.InitializeBarrier(Barrier->GetLockController(FIRST));
	LockController2.InitializeBarrier(Barrier->GetLockController(SECOND));
	RangeController1.InitializeBarrier(Barrier->GetRangeController(FIRST));
	RangeController2.InitializeBarrier(Barrier->GetRangeController(SECOND));
	TargetSpeedController1.InitializeBarrier(Barrier->GetTargetSpeedController(FIRST));
	TargetSpeedController2.InitializeBarrier(Barrier->GetTargetSpeedController(SECOND));
	ScrewController.InitializeBarrier(Barrier->GetScrewController());
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
	ScrewController.UpdateNativeProperties();
}
