#include "ControllerConstraintBarriers.h"

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"


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