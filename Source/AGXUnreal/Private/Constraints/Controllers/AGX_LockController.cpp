#include "Constraints/Controllers/AGX_LockController.h"

#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintLockController::FAGX_ConstraintLockController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Position(0.0)
{
}

void FAGX_ConstraintLockController::InitializeBarrier(TUniquePtr<FLockControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FLockControllerBarrier* GetLockBarrier(FAGX_ConstraintLockController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FLockControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintLockController::SetPosition(double InPosisiton)
{
	if (HasNative())
	{
		if (bRotational)
		{
			GetLockBarrier(*this)->SetPositionRotational(InPosisiton);
		}
		else
		{
			GetLockBarrier(*this)->SetPositionTranslational(InPosisiton);
		}
	}
	Position = InPosisiton;
}

double FAGX_ConstraintLockController::GetPosition()
{
	if (HasNative())
	{
		if (bRotational)
		{
			return GetLockBarrier(*this)->GetPositionRotational();
		}
		else
		{
			return GetLockBarrier(*this)->GetPositionTranslational();
		}
	}
	return Position;
}

void FAGX_ConstraintLockController::UpdateNativePropertiesImpl()
{
	FLockControllerBarrier* Barrier = GetLockBarrier(*this);
	check(Barrier);
	if (bRotational)
	{
		Barrier->SetPositionRotational(Position);
	}
	else
	{
		Barrier->SetPositionTranslational(Position);
	}
}

void FAGX_ConstraintLockController::CopyFrom(const FLockControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	if (bRotational)
	{
		Position = Source.GetPositionRotational();
	}
	else
	{
		Position = Source.GetPositionTranslational();
	}
}

#if 0
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

void FAGX_ConstraintLockController::FromBarrier(FLockControllerBarrier& Barrier)
{
	bEnable = Barrier.bEnable;
	Elasticity = Barrier.Elasticity;
	Damping = Barrier.Damping;
	ForceRange.Min = Barrier.ForceRangeMin;
	ForceRange.Max = Barrier.ForceRangeMax;

	bRotational = Barrier.bRotational;

	Position = bRotational ? FMath::RadiansToDegrees(Barrier.Position) : Barrier.Position;
}
#endif
