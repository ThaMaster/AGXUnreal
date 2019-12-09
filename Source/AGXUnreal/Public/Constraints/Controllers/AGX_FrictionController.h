#pragma once

#include "CoreMinimal.h"

#include "AGX_FrictionController.generated.h"

/**
 * Friction controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintFrictionController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/**
	 * Note that if this controller is rotational (Hinge or CylindriclJoint)
	 * the radius (meters) of the axle should be included in the friction coefficient
	 * for the comparisons with the normal force to be dimensionally correct.
	 * I.e., friction_torque <= friction_coefficient * axle_radius * normal_force
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double FrictionCoefficient;

	/**
	 * Enable/disable non-linear update of the friction conditions given
	 * current normal force from the direct solver. When enabled - this
	 * feature is similar to scale box friction models with solve type DIRECT.
	 *
	 * Note that this feature only supports constraint solve types DIRECT and
	 * DIRECT_AND_ITERATIVE - meaning, if the constraint has solve
	 * type ITERATIVE, this feature is ignored.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	bool bEnableNonLinearDirectSolveUpdate;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintFrictionController(bool bRotational = false);

	void ToBarrier(struct FFrictionControllerBarrier* Barrier) const;

private:
	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
