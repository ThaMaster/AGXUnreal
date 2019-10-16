#include "ControllerConstraintBarriers.h"

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"


void FFrictionControllerBarrier::ToNative(agx::FrictionController* Native, UWorld* World) const
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


void FRangeControllerBarrier::ToNative(agx::RangeController* Native, UWorld* World) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setRange(agx::RangeReal(
		bRotational ? RangeMin : ConvertDistanceToAgx(RangeMin, World),
		bRotational ? RangeMax : ConvertDistanceToAgx(RangeMax, World)));
}


void FTargetSpeedControllerBarrier::ToNative(agx::TargetSpeedController* Native, UWorld* World) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setSpeed(bRotational ? Speed : ConvertDistanceToAgx(Speed, World));
	Native->setLockedAtZeroSpeed(bLockedAtZeroSpeed);
}