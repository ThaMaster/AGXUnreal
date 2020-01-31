#pragma once

#include "CoreMinimal.h"

#include "AGX_ConstraintController.generated.h"

class FConstraintControllerBarrier;

/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintController();

	TUniquePtr<FConstraintControllerBarrier> NativeBarrier;

private:
	/// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
