#pragma once

#include "CoreMinimal.h"

#include "AGX_ElectricMotorController.generated.h"

struct FElectricMotorControllerBarrier;

/**
 * Electric motor controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintElectricMotorController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/**
	 * Available voltage or voltage drop across the terminals of this motor, in Volt.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Voltage;

	/**
	 * Resistance in the armature circuit, in Ohm.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double ArmatureResistance;

	/**
	 * Torque constant of this motor, in unit Torque Per Ampere. This value
	 * couples the torque out to current in.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double TorqueConstant;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintElectricMotorController(bool bRotational = false);

	void ToBarrier(FElectricMotorControllerBarrier* Barrier) const;
	void FromBarrier(const FElectricMotorControllerBarrier& Barrier);

private:
	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
