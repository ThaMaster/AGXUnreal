#include "AGX_TargetSpeedController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintTargetSpeedController::FAGX_ConstraintTargetSpeedController(bool bRotational_)
	:
	bEnable(false),
	Speed(0.0),
	bLockedAtZeroSpeed(false),
	Elasticity(1.0e8),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintTargetSpeedController::ToBarrier(FTargetSpeedControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Speed = bRotational ? FMath::DegreesToRadians(Speed) : Speed;
	Barrier->bLockedAtZeroSpeed = bLockedAtZeroSpeed;
}
