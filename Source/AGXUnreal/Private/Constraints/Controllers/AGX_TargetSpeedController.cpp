#include "Constraints/Controllers/AGX_TargetSpeedController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintTargetSpeedController::FAGX_ConstraintTargetSpeedController(bool bRotational_)
	: bEnable(false)
	, Speed(0.0)
	, bLockedAtZeroSpeed(false)
	, Elasticity(ConstraintConstants::DefaultElasticity())
	, Damping(ConstraintConstants::DefaultDamping())
	, ForceRange(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax())
	, bRotational(bRotational_)
{
}

void FAGX_ConstraintTargetSpeedController::ToBarrier(FTargetSpeedControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = static_cast<double>(ForceRange.Min);
	Barrier->ForceRangeMax = static_cast<double>(ForceRange.Max);

	Barrier->bRotational = bRotational;

	Barrier->Speed = bRotational ? FMath::DegreesToRadians(Speed) : Speed;
	Barrier->bLockedAtZeroSpeed = bLockedAtZeroSpeed;
}

void FAGX_ConstraintTargetSpeedController::FromBarrier(const FTargetSpeedControllerBarrier& Barrier)
{
	bEnable = Barrier.bEnable;
	Elasticity = Barrier.Elasticity;
	Damping = Barrier.Damping;
	ForceRange.Min = static_cast<float>(Barrier.ForceRangeMin);
	ForceRange.Max = static_cast<float>(Barrier.ForceRangeMax);

	bRotational = Barrier.bRotational;

	Speed = bRotational ? FMath::RadiansToDegrees(Barrier.Speed) : Barrier.Speed;
	bLockedAtZeroSpeed = Barrier.bLockedAtZeroSpeed;
}
