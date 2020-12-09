#include "Constraints/Controllers/AGX_ElectricMotorController.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintElectricMotorController::FAGX_ConstraintElectricMotorController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Voltage(24.0)
	, ArmatureResistance(1.0)
	, TorqueConstant(1.0)
{
}

void FAGX_ConstraintElectricMotorController::InitializeBarrier(
	TUniquePtr<FElectricMotorControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
}

namespace
{
	FElectricMotorControllerBarrier* GetElectricMotorBarrier(
		FAGX_ConstraintElectricMotorController& Controller)
	{
		// Is there a way to guarantee that this cast is safe? We're in the
		// Unreal Engine potion of the plugin so cannot use dynamic_cast, but
		// FElectricMotorControllerBarrier doesn't inherit from UObject so we
		// cannot use Unreal Engine's Cast<> function.
		//
		// We "know" that a FAGX_ConstraintElectricMotorController will always
		// hold a FElectricMotorControllerBarrier, but there doesn't seem to be
		// a way to verify it here.
		//
		// The corresponding functions for the other ControllerBarriers reference
		// this comment. Remove those if this comment is removed.
		return static_cast<FElectricMotorControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintElectricMotorController::UpdateNativePropertiesImpl()
{
	FElectricMotorControllerBarrier* Barrier = GetElectricMotorBarrier(*this);
	check(Barrier);
	Barrier->SetVoltage(Voltage);
	Barrier->SetArmatureResistance(ArmatureResistance);
	Barrier->SetTorqueConstant(TorqueConstant);
}

void FAGX_ConstraintElectricMotorController::CopyFrom(const FElectricMotorControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	Voltage = Source.GetVoltage();
	ArmatureResistance = Source.GetArmatureResistance();
	TorqueConstant = Source.GetTorqueConstant();
}
