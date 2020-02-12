#include "Constraints/Controllers/AGX_FrictionController.h"

#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintFrictionController::FAGX_ConstraintFrictionController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, FrictionCoefficient(0.416667)
	, bEnableNonLinearDirectSolveUpdate(false)
{
}

void FAGX_ConstraintFrictionController::InitializeBarrier(TUniquePtr<FFrictionControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FFrictionControllerBarrier* GetFrictionBarrier(FAGX_ConstraintFrictionController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FFrictionControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintFrictionController::UpdateNativePropertiesImpl()
{
	FFrictionControllerBarrier* Barrier = GetFrictionBarrier(*this);
	check(Barrier);
	Barrier->SetFrictionCoefficient(FrictionCoefficient);
	Barrier->SetEnableNonLinearDirectSolveUpdate(bEnableNonLinearDirectSolveUpdate);
}

void FAGX_ConstraintFrictionController::CopyFrom(const FFrictionControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	FrictionCoefficient = Source.GetFrictionCoefficient();
	bEnableNonLinearDirectSolveUpdate = Source.GetEnableNonLinearDirectSolveUpdate();
}
