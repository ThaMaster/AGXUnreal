#include "Constraints/Controllers/AGX_TargetSpeedController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintTargetSpeedController::FAGX_ConstraintTargetSpeedController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Speed(0.0)
	, bLockedAtZeroSpeed(false)
{
}

void FAGX_ConstraintTargetSpeedController::InitializeBarrier(TUniquePtr<FTargetSpeedControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FTargetSpeedControllerBarrier* GetSpeedBarrier(FAGX_ConstraintTargetSpeedController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FTargetSpeedControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintTargetSpeedController::SetSpeed(float InSpeed)
{
	Speed = static_cast<double>(InSpeed);
	Super::UpdateNativeProperties();
}

void FAGX_ConstraintTargetSpeedController::UpdateNativePropertiesImpl()
{
	FTargetSpeedControllerBarrier* Barrier = GetSpeedBarrier(*this);
	check(Barrier);
	Barrier->SetLockedAtZeroSpeed(bLockedAtZeroSpeed);
	if (bRotational)
	{
		Barrier->SetSpeedRotational(Speed);
	}
	else
	{
		Barrier->SetSpeedTranslational(Speed);
	}
}

void FAGX_ConstraintTargetSpeedController::CopyFrom(const FTargetSpeedControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	bLockedAtZeroSpeed = Source.GetLockedAtZeroSpeed();
	if (bRotational)
	{
		Speed = Source.GetSpeedRotational();
	}
	else
	{
		Speed = Source.GetSpeedTranslational();
	}
}
