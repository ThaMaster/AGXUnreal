#pragma once

#include "CoreMinimal.h"

#include "AGX_LockController.generated.h"

struct FLockControllerBarrier;

/**
 * Lock controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintLockController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/**
	 * Target position in Degrees if controller is on a Rotational Degree-Of-Freedom,
	 * else in Centimeters. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Position;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintLockController(bool bRotational = false);

	void ToBarrier(FLockControllerBarrier* Barrier) const;
	void FromBarrier(FLockControllerBarrier& Barrier);

private:
	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
