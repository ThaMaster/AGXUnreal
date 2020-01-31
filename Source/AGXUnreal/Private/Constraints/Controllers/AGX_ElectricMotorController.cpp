#include "Constraints/Controllers/AGX_ElectricMotorController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintElectricMotorController::FAGX_ConstraintElectricMotorController(bool bRotational_)
	: bEnable(false)
	, Voltage(24.0)
	, ArmatureResistance(1.0)
	, TorqueConstant(1.0)
	, ForceRange(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax())
	, bRotational(bRotational_)
{
}

void FAGX_ConstraintElectricMotorController::ToBarrier(
	FElectricMotorControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Voltage = Voltage;
	Barrier->ArmatureResistance = ArmatureResistance;
	Barrier->TorqueConstant = TorqueConstant;
}

void FAGX_ConstraintElectricMotorController::FromBarrier(const FElectricMotorControllerBarrier& Barrier)
{
	bEnable = Barrier.bEnable;
	ForceRange.Min = Barrier.ForceRangeMin;
	ForceRange.Max = Barrier.ForceRangeMax;

	bRotational = Barrier.bRotational;

	Voltage = Barrier.Voltage;
	ArmatureResistance = Barrier.ArmatureResistance;
	TorqueConstant = Barrier.TorqueConstant;
}
