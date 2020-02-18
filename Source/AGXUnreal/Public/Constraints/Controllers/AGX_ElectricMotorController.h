#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ElectricMotorController.generated.h"

class FElectricMotorControllerBarrier;

/**
 * Electric motor controller for secondary constraints (usually on one of the
 * DOFs that has not been primarily constrained by the AGX Constraint). Disabled
 * by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintElectricMotorController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Available voltage or voltage drop across the terminals of this motor, in Volt.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller", Meta = (EditCondition = "bEnable"))
	double Voltage;

	/**
	 * Resistance in the armature circuit, in Ohm.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller",
		Meta = (EditCondition = "bEnable"))
	double ArmatureResistance;

	/**
	 * Torque constant of this motor, in unit Torque Per Ampere. This value
	 * couples the torque out to current in.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller",
		Meta = (EditCondition = "bEnable"))
	double TorqueConstant;

public:
	FAGX_ConstraintElectricMotorController() = default;
	FAGX_ConstraintElectricMotorController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FElectricMotorControllerBarrier> Barrier);
	void CopyFrom(const FElectricMotorControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};
