#include "Constraints/Controllers/AGX_RangeController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintRangeController::FAGX_ConstraintRangeController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Range(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax())
{
}

void FAGX_ConstraintRangeController::InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FRangeControllerBarrier* GetRangeBarrier(FAGX_ConstraintRangeController& Controller)
	{
		// See comment in GetElectricMotorBarrier.
		return static_cast<FRangeControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintRangeController::UpdateNativePropertiesImpl()
{
	FRangeControllerBarrier* Barrier = GetRangeBarrier(*this);
	check(Barrier);
	if (bRotational)
	{
		Barrier->SetRangeRotational(Range);
	}
	else
	{
		Barrier->SetRangeTranslational(Range);
	}
}

void FAGX_ConstraintRangeController::CopyFrom(const FRangeControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	if (bRotational)
	{
		Range = Source.GetRangeRotational();
	}
	else
	{
		Range = Source.GetRangeTranslational();
	}
}
