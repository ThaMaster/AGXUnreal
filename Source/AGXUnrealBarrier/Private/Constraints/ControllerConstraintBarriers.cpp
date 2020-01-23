#include "Constraints/ControllerConstraintBarriers.h"

#include "TypeConversions.h"

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

void FElectricMotorControllerBarrier::ToNative(agx::ElectricMotorController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setVoltage(Voltage);
	Native->setArmatureResistance(ArmatureResistance);
	Native->setTorqueConstant(TorqueConstant);
}

void FFrictionControllerBarrier::ToNative(agx::FrictionController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setFrictionCoefficient(FrictionCoefficient);
	Native->setEnableNonLinearDirectSolveUpdate(bEnableNonLinearDirectSolveUpdate);
}

void FLockControllerBarrier::ToNative(agx::LockController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setPosition(bRotational ? Position : ConvertDistanceToAgx(Position));
}

void FRangeControllerBarrier::ToNative(agx::RangeController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setRange(agx::RangeReal(
		bRotational ? RangeMin : ConvertDistanceToAgx(RangeMin),
		bRotational ? RangeMax : ConvertDistanceToAgx(RangeMax)));
}

void FScrewControllerBarrier::ToNative(agx::ScrewController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setLead(ConvertDistanceToAgx(Lead));
}

void FTargetSpeedControllerBarrier::ToNative(agx::TargetSpeedController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setSpeed(bRotational ? Speed : ConvertDistanceToAgx(Speed));
	Native->setLockedAtZeroSpeed(bLockedAtZeroSpeed);
}
