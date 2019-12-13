#pragma once

#include "CoreMinimal.h"

#include "AGX_RangeController.generated.h"

/**
 * Range controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangeController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/** Range in Degrees if controller is on a Rotational Degree-Of-Freedom,  else in Centimeters.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval Range;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintRangeController(bool bRotational = false);

	void ToBarrier(struct FRangeControllerBarrier* Barrier) const;

private:
	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
