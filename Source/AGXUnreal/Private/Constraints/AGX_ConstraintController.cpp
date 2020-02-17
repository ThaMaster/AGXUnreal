#include "Constraints/AGX_ConstraintController.h"

#include "Constraints/AGX_ConstraintConstants.h"

#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintController::FAGX_ConstraintController()
	// This 'false' here doesn't really make much sense, but we must have a
	// default constructor and the default constructor must provide some value.
	// Should never be called for an actual ConstraintController.
	: FAGX_ConstraintController(false)
{
}

FAGX_ConstraintController::FAGX_ConstraintController(bool bInRotational)
	: bEnable(false)
	, Compliance(ConstraintConstants::DefaultCompliance())
	, Damping(ConstraintConstants::DefaultDamping())
	, ForceRange(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax())
	, bRotational(bInRotational)
	, NativeBarrier(nullptr)
{
}

FAGX_ConstraintController::~FAGX_ConstraintController()
{

}

FAGX_ConstraintController& FAGX_ConstraintController::operator=(const FAGX_ConstraintController& Other)
{
	bEnable = Other.bEnable;
	Compliance = Other.Compliance;
	Damping = Other.Damping;
	ForceRange = Other.ForceRange;
	bRotational = Other.bRotational;
	return *this;
}

bool FAGX_ConstraintController::HasNative() const
{
	return NativeBarrier.IsValid() && NativeBarrier->HasNative();
}

FConstraintControllerBarrier* FAGX_ConstraintController::GetNative()
{
	check(HasNative());
	return NativeBarrier.Get();
}

void FAGX_ConstraintController::UpdateNativeProperties()
{
	check(HasNative());
	NativeBarrier->SetEnable(bEnable);
	NativeBarrier->SetCompliance(Compliance);
	NativeBarrier->SetDamping(Damping);
	NativeBarrier->SetForceRange(ForceRange);
	UpdateNativePropertiesImpl();
}

void FAGX_ConstraintController::CopyFrom(const FConstraintControllerBarrier& Source)
{
	bEnable = Source.GetEnable();
	Compliance = Source.GetCompliance();
	Damping = Source.GetDamping();
	ForceRange = Source.GetForceRange();
}
