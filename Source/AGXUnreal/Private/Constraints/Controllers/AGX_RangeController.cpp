#include "AGX_RangeController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintRangeController::FAGX_ConstraintRangeController(bool bRotational_)
	:
	bEnable(false),
	Range({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	Elasticity(DEFAULT_SECONDARY_ELASTICITY),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ 0.0, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintRangeController::ToBarrier(FRangeControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->RangeMin = bRotational ? FMath::DegreesToRadians(Range.Min) : Range.Min;
	Barrier->RangeMax = bRotational ? FMath::DegreesToRadians(Range.Max) : Range.Max;
}

