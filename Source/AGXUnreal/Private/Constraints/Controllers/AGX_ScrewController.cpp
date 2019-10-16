#include "AGX_ScrewController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


FAGX_ConstraintScrewController::FAGX_ConstraintScrewController(bool bRotational_)
	:
	bEnable(false),
	Lead(0.0),
	Elasticity(ConstraintConstants::DefaultElasticity()),
	Damping(ConstraintConstants::DefaultDamping()),
	ForceRange({ ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax() })
{

}


void FAGX_ConstraintScrewController::ToBarrier(FScrewControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->Lead = Lead;
}
