// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/Constraint2DOFBarrier.h"

#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

FConstraint2DOFBarrier::FConstraint2DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint2DOFBarrier::FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint2DOFBarrier::~FConstraint2DOFBarrier()
{
}

namespace
{
	template <typename NativeController>
	using ControllerGetter2DOF = std::function<const NativeController*(const agx::Constraint2DOF*)>;

	template <typename NativeController, typename ControllerBarrier>
	void GetController2DOF(
		const agx::Constraint2DOF* Constraint, ControllerBarrier& Controller,
		ControllerGetter2DOF<NativeController> Getter)
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

void FConstraint2DOFBarrier::SetElectricMotorController(
	const FElectricMotorControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::ElectricMotorController* NativeController = NativeCasted->getElectricMotorController(
			(agx::Constraint2DOF::DOF) SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetElectricMotorController(
	FElectricMotorControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex) const
{
	GetController2DOF<agx::ElectricMotorController>(
		GetNativeCasted(), ControllerBarrier,
		[SecondaryConstraintIndex](const agx::Constraint2DOF* Constraint) {
			return Constraint->getElectricMotorController(
				(agx::Constraint2DOF::DOF) SecondaryConstraintIndex);
		});
}

void FConstraint2DOFBarrier::SetFrictionController(
	const FFrictionControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::FrictionController* NativeController = NativeCasted->getFrictionController(
			(agx::Constraint2DOF::DOF) SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetFrictionController(
	FFrictionControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex) const
{
	GetController2DOF<agx::FrictionController>(
		GetNativeCasted(), ControllerBarrier,
		[SecondaryConstraintIndex](const agx::Constraint2DOF* Constraint) {
			return Constraint->getFrictionController(
				(agx::Constraint2DOF::DOF) SecondaryConstraintIndex);
		});
}

void FConstraint2DOFBarrier::SetLockController(
	const FLockControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::LockController* NativeController =
			NativeCasted->getLock1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetLockController(
	FLockControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex) const
{
	GetController2DOF<agx::LockController>(
		GetNativeCasted(), ControllerBarrier,
		[SecondaryConstraintIndex](const agx::Constraint2DOF* Constraint) {
			return Constraint->getLock1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);
		});
}

void FConstraint2DOFBarrier::SetRangeController(
	const FRangeControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::RangeController* NativeController =
			NativeCasted->getRange1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetRangeController(
	FRangeControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex) const
{
	GetController2DOF<agx::RangeController>(
		GetNativeCasted(), ControllerBarrier,
		[SecondaryConstraintIndex](const agx::Constraint2DOF* Constraint) {
			return Constraint->getRange1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);
		});
}

void FConstraint2DOFBarrier::SetTargetSpeedController(
	const FTargetSpeedControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::TargetSpeedController* NativeController =
			NativeCasted->getMotor1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetTargetSpeedController(
	FTargetSpeedControllerBarrier& ControllerBarrier, int32 SecondaryConstraintIndex) const
{
	GetController2DOF<agx::TargetSpeedController>(
		GetNativeCasted(), ControllerBarrier,
		[SecondaryConstraintIndex](const agx::Constraint2DOF* Constraint) {
			return Constraint->getMotor1D((agx::Constraint2DOF::DOF) SecondaryConstraintIndex);
		});
}

void FConstraint2DOFBarrier::SetScrewController(const FScrewControllerBarrier& ControllerBarrier)
{
	if (agx::Constraint2DOF* NativeCasted = GetNativeCasted())
	{
		agx::ScrewController* NativeController = NativeCasted->getScrew1D();

		ControllerBarrier.ToNative(NativeController);
	}
}

void FConstraint2DOFBarrier::GetScrewController(FScrewControllerBarrier& ControllerBarrier) const
{
	GetController2DOF<agx::ScrewController>(
		GetNativeCasted(), ControllerBarrier,
		[](const agx::Constraint2DOF* Constraint) { return Constraint->getScrew1D(); });
}

agx::Constraint2DOF* FConstraint2DOFBarrier::GetNativeCasted() const
{
	if (HasNative())
		return static_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	else
		return nullptr;
}
