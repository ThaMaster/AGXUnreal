#include "Constraints/AGX_Constraint2DofComponent.h"

// AGX Dynamics for Unreal includes.
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

namespace AGX_Constraint2DofComponent_helpers
{
	void InitializeControllerBarriers(UAGX_Constraint2DofComponent& Constraint)
	{
		FConstraint2DOFBarrier* Barrier = Get2DofBarrier(Constraint);

		EAGX_Constraint2DOFFreeDOF FIRST = EAGX_Constraint2DOFFreeDOF::FIRST;
		EAGX_Constraint2DOFFreeDOF SECOND = EAGX_Constraint2DOFFreeDOF::SECOND;

		Constraint.ElectricMotorController1.InitializeBarrier(Barrier->GetElectricMotorController(FIRST));
		Constraint.FrictionController1.InitializeBarrier(Barrier->GetFrictionController(FIRST));
		Constraint.LockController1.InitializeBarrier(Barrier->GetLockController(FIRST));
		Constraint.RangeController1.InitializeBarrier(Barrier->GetRangeController(FIRST));
		Constraint.TargetSpeedController1.InitializeBarrier(Barrier->GetTargetSpeedController(FIRST));

		Constraint.ElectricMotorController2.InitializeBarrier(Barrier->GetElectricMotorController(SECOND));
		Constraint.FrictionController2.InitializeBarrier(Barrier->GetFrictionController(SECOND));
		Constraint.LockController2.InitializeBarrier(Barrier->GetLockController(SECOND));
		Constraint.RangeController2.InitializeBarrier(Barrier->GetRangeController(SECOND));
		Constraint.TargetSpeedController2.InitializeBarrier(Barrier->GetTargetSpeedController(SECOND));

		Constraint.ScrewController.InitializeBarrier(Barrier->GetScrewController());
	}
}

void UAGX_Constraint2DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint2DofComponent_helpers::InitializeControllerBarriers(*this);
}

void UAGX_Constraint2DofComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Constraint '%s' is trying to update native properties while not having a "
				 "native handle."),
			*GetName());
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

void UAGX_Constraint2DofComponent::AssignNative(uint64 NativeAddress)
{
	Super::AssignNative(NativeAddress);
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint2DofComponent_helpers::InitializeControllerBarriers(*this);
}
