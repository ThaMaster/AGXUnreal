#include "Utilities/AGX_ConstraintUtilities.h"

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint1DofComponent& Component, const FConstraint1DOFBarrier& Barrier)
{
	StoreElectricMotorController(Barrier, Component.ElectricMotorController);
	StoreFrictionController(Barrier, Component.FrictionController);
	StoreLockController(Barrier, Component.LockController);
	StoreRangeController(Barrier, Component.RangeController);
	StoreTargetSpeedController(Barrier, Component.TargetSpeedController);
}

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint2DofComponent& Component, const FConstraint2DOFBarrier& Barrier)
{
	const EAGX_Constraint2DOFFreeDOF First = EAGX_Constraint2DOFFreeDOF::FIRST;
	const EAGX_Constraint2DOFFreeDOF Second = EAGX_Constraint2DOFFreeDOF::SECOND;

	StoreElectricMotorController(Barrier, Component.ElectricMotorController1, First);
	StoreElectricMotorController(Barrier, Component.ElectricMotorController2, Second);

	StoreFrictionController(Barrier, Component.FrictionController1, First);
	StoreFrictionController(Barrier, Component.FrictionController2, Second);

	StoreLockController(Barrier, Component.LockController1, First);
	StoreLockController(Barrier, Component.LockController2, Second);

	StoreRangeController(Barrier, Component.RangeController1, First);
	StoreRangeController(Barrier, Component.RangeController2, Second);

	StoreTargetSpeedController(Barrier, Component.TargetSpeedController1, First);
	StoreTargetSpeedController(Barrier, Component.TargetSpeedController2, Second);
}

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_ConstraintComponent& Component, const FConstraintBarrier& Barrier)
{
	// This exists only to provide a complete overload resolution set for the constraint types. Only
	// 1Dof and 2Dof constraints, the other overloads, have any controllers to store. Make sure you
	// end up in the correct overload of CopyControllersFrom for the type of constraint you actually
	// have.
	/// \todo Consider making this a virtual member function instead, to avoid ending up in this
	/// empty version unintentionally.
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller)
{
	Controller.CopyFrom(*Barrier.GetElectricMotorController());
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetElectricMotorController(Dof));
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller)
{
	Controller.CopyFrom(*Barrier.GetFrictionController());
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetFrictionController(Dof));
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller)
{
	Controller.CopyFrom(*Barrier.GetLockController());
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetLockController(Dof));
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller)
{
	Controller.CopyFrom(*Barrier.GetRangeController());
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetRangeController(Dof));
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController());
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController(Dof));
}

void FAGX_ConstraintUtilities::StoreFrame(
	const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment, int32 BodyIndex)
{
	Attachment.FrameDefiningComponent.Clear();
	Attachment.LocalFrameLocation = Barrier.GetLocalLocation(BodyIndex);
	Attachment.LocalFrameRotation = Barrier.GetLocalRotation(BodyIndex);
}

void FAGX_ConstraintUtilities::StoreFrames(
	const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component)
{
	StoreFrame(Barrier, Component.BodyAttachment1, 0);
	StoreFrame(Barrier, Component.BodyAttachment2, 1);
}
