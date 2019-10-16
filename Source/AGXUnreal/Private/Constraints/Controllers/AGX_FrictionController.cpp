#include "AGX_FrictionController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintFrictionController::FAGX_ConstraintFrictionController(bool bRotational_)
	:
	bEnable(false),
	FrictionCoefficient(0.416667),
	bEnableNonLinearDirectSolveUpdate(false),
	Elasticity(ConstraintConstants::DefaultElasticity()),
	Damping(ConstraintConstants::DefaultDamping()),
	ForceRange({ ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax() }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintFrictionController::ToBarrier(FFrictionControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->FrictionCoefficient = FrictionCoefficient;
	Barrier->bEnableNonLinearDirectSolveUpdate = bEnableNonLinearDirectSolveUpdate;
}
