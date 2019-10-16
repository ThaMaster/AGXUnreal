#include "AGX_ElectricMotorController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintElectricMotorController::FAGX_ConstraintElectricMotorController(bool bRotational_)
	:
	bEnable(false),
	Voltage(24.0),
	ArmatureResistance(1.0),
	TorqueConstant(1.0),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintElectricMotorController::ToBarrier(FElectricMotorControllerBarrier* Barrier) const
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
