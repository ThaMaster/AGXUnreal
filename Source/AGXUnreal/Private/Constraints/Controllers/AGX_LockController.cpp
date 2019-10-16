#include "AGX_LockController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintLockController::FAGX_ConstraintLockController(bool bRotational_)
	:
	bEnable(false),
	Position(0.0),
	Elasticity(1.0e8),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintLockController::ToBarrier(FLockControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Position = bRotational ? FMath::DegreesToRadians(Position) : Position;
}
