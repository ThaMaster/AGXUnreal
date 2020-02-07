//

#pragma once

#include "CoreMinimal.h"

#include "AGX_RangeController.generated.h"

struct FRangeControllerBarrier;

/**
 * Range controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangeController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Range Controller")
	bool bEnable;

	/** Range in Degrees if controller is on a Rotational Degree-Of-Freedom,  else in Centimeters.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	FFloatInterval Range;

	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintRangeController(bool bRotational = false);

	void ToBarrier(FRangeControllerBarrier* Barrier) const;
	void FromBarrier(FRangeControllerBarrier& Barrier);

private:
	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
