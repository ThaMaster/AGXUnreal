#include "Constraints/Controllers/AGX_ScrewController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintScrewController::FAGX_ConstraintScrewController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Lead(0.0)
{
}

void FAGX_ConstraintScrewController::InitializeBarrier(TUniquePtr<FScrewControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FScrewControllerBarrier* GetScrewBarrier(FAGX_ConstraintScrewController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FScrewControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintScrewController::UpdateNativePropertiesImpl()
{
	FScrewControllerBarrier* Barrier = GetScrewBarrier(*this);
	check(Barrier);
	Barrier->SetLead(Lead);
}

void FAGX_ConstraintScrewController::CopyFrom(const FScrewControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	Lead = Source.GetLead();
}
