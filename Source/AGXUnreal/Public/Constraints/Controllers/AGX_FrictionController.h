#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_FrictionController.generated.h"

class FFrictionControllerBarrier;

/**
 * Friction controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint). Disabled by
 * default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintFrictionController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

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

public:
	FAGX_ConstraintFrictionController() = default;
	FAGX_ConstraintFrictionController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FFrictionControllerBarrier> Barrier);
	void CopyFrom(const FFrictionControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};
