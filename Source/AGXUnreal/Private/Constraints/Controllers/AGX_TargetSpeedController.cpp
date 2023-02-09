// Copyright 2023, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TargetSpeedController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintTargetSpeedController::FAGX_ConstraintTargetSpeedController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
{
}

void FAGX_ConstraintTargetSpeedController::InitializeBarrier(
	TUniquePtr<FTargetSpeedControllerBarrier> Barrier)
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

	const FTargetSpeedControllerBarrier* GetSpeedBarrier(
		const FAGX_ConstraintTargetSpeedController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<const FTargetSpeedControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintTargetSpeedController::SetSpeed(double InSpeed)
{
	if (HasNative())
	{
		if (bRotational)
		{
			GetSpeedBarrier(*this)->SetSpeedRotational(InSpeed);
		}
		else
		{
			GetSpeedBarrier(*this)->SetSpeedTranslational(InSpeed);
		}
	}
	Speed = InSpeed;
}

double FAGX_ConstraintTargetSpeedController::GetSpeed() const
{
	if (HasNative())
	{
		if (bRotational)
		{
			return GetSpeedBarrier(*this)->GetSpeedRotational();
		}
		else
		{
			return GetSpeedBarrier(*this)->GetSpeedTranslational();
		}
	}
	return Speed;
}

void FAGX_ConstraintTargetSpeedController::SetLockedAtZeroSpeed(bool bInLockedAtZeroSpeed)
{
	if (HasNative())
	{
		GetSpeedBarrier(*this)->SetLockedAtZeroSpeed(bInLockedAtZeroSpeed);
	}
	bLockedAtZeroSpeed = bInLockedAtZeroSpeed;
}

bool FAGX_ConstraintTargetSpeedController::GetLockedAtZeroSpeed() const
{
	if (HasNative())
	{
		return GetSpeedBarrier(*this)->GetLockedAtZeroSpeed();
	}
	else
	{
		return bLockedAtZeroSpeed;
	}
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

void FAGX_ConstraintTargetSpeedController::CopyFrom(
	const FTargetSpeedControllerBarrier& Source,
	TArray<FAGX_ConstraintTargetSpeedController*>& Instances, bool ForceOverwriteInstances)
{
	TArray<FAGX_ConstraintController*> BaseInstances;
	BaseInstances.Reserve(Instances.Num());
	for (auto Instance : Instances)
	{
		BaseInstances.Add(Instance);
	}	

	Super::CopyFrom(Source, BaseInstances, ForceOverwriteInstances);
	const double SpeedBarrier =
		bRotational ? Source.GetSpeedRotational() : Source.GetSpeedTranslational();

	for (auto Instance : Instances)
	{
		if (Instance == nullptr)
			continue;

		if (ForceOverwriteInstances || Instance->bLockedAtZeroSpeed == bLockedAtZeroSpeed)
		{
			Instance->bLockedAtZeroSpeed = Source.GetLockedAtZeroSpeed();
		}

		if (ForceOverwriteInstances || Instance->Speed == Speed)
		{
			Instance->Speed = SpeedBarrier;
		}
	}

	bLockedAtZeroSpeed = Source.GetLockedAtZeroSpeed();
	Speed = SpeedBarrier;
}
