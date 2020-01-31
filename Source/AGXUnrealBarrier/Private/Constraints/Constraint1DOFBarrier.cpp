// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/Constraint1DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

FConstraint1DOFBarrier::FConstraint1DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint1DOFBarrier::FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint1DOFBarrier::~FConstraint1DOFBarrier()
{
}

namespace
{
	template <typename NativeController>
	using ControllerGetter = std::function<const NativeController*(const agx::Constraint1DOF*)>;

	template <typename NativeController, typename ControllerBarrier>
	void GetController(
		const agx::Constraint1DOF* Constraint, ControllerBarrier& Controller,
		ControllerGetter<NativeController> Getter)
	{
		if (Constraint == nullptr)
		{
			return;
		}

		const NativeController* ControllerAGX = Getter(Constraint);
		if (ControllerAGX == nullptr)
		{
			return;
		}

		Controller.FromNative(*ControllerAGX);
	}
}

void FConstraint1DOFBarrier::SetElectricMotorController(
	const FElectricMotorControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::ElectricMotorController* NativeController = NativeCasted->getElectricMotorController();
		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::GetElectricMotorController(
	FElectricMotorControllerBarrier& ControllerBarrier) const
{
	GetController<agx::ElectricMotorController>(
		GetNativeCasted(), ControllerBarrier, [](const agx::Constraint1DOF* Constraint) {
			return Constraint->getElectricMotorController();
		});
}

void FConstraint1DOFBarrier::SetFrictionController(
	const FFrictionControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::FrictionController* NativeController = NativeCasted->getFrictionController();
		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::GetFrictionController(
	FFrictionControllerBarrier& ControllerBarrier) const
{
	GetController<agx::FrictionController>(
		GetNativeCasted(), ControllerBarrier,
		[](const agx::Constraint1DOF* Constraint) { return Constraint->getFrictionController(); });
}

void FConstraint1DOFBarrier::SetLockController(const FLockControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::LockController* NativeController = NativeCasted->getLock1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::GetLockController(FLockControllerBarrier& ControllerBarrier) const
{
	GetController<agx::LockController>(
		GetNativeCasted(), ControllerBarrier,
		[](const agx::Constraint1DOF* Constraint) { return Constraint->getLock1D(); });
}

void FConstraint1DOFBarrier::SetRangeController(const FRangeControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::RangeController* NativeController = NativeCasted->getRange1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::GetRangeController(FRangeControllerBarrier& ControllerBarrier) const
{
	GetController<agx::RangeController>(GetNativeCasted(), ControllerBarrier,
		[](const agx::Constraint1DOF* Constraint) { return Constraint->getRange1D(); });
}

void FConstraint1DOFBarrier::SetTargetSpeedController(
	const FTargetSpeedControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint1DOF* NativeCasted = GetNativeCasted())
	{
		agx::TargetSpeedController* NativeController = NativeCasted->getMotor1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint1DOFBarrier::GetTargetSpeedController(FTargetSpeedControllerBarrier& ControllerBarrier) const
{
	GetController<agx::TargetSpeedController>(GetNativeCasted(), ControllerBarrier,
		[](const agx::Constraint1DOF* Constraint) { return Constraint->getMotor1D(); });
}

agx::Constraint1DOF* FConstraint1DOFBarrier::GetNativeCasted() const
{
	if (HasNative())
		return static_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	else
		return nullptr;
}
