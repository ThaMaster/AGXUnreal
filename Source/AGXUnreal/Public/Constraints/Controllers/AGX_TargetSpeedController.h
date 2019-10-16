#pragma once

#include "CoreMinimal.h"

#include "AGX_TargetSpeedController.generated.h"


/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintTargetSpeedController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/**
	 * Target Speed in Degrees Per Second if controller is on a Rotational DOF,
	 * else in Centimeters Per Second.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Speed;

	/**
	 * Whether the controller should auto-lock whenever target speed is zero,
	 * such that it will not drift away from that angle/position if the assigned
	 * force range is enough to hold it.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	bool bLockedAtZeroSpeed;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:

	FAGX_ConstraintTargetSpeedController(bool bRotational = false);

	void ToBarrier(struct FTargetSpeedControllerBarrier* Barrier) const;

private:

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
