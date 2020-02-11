#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_RangeController.generated.h"

class FRangeControllerBarrier;

/**
 * Range controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangeController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Range in Degrees if controller is on a Rotational Degree-Of-Freedom,
	 * else in Centimeters.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	//TInterval<double> Range;
	FFloatInterval Range;

public:
	FAGX_ConstraintRangeController() = default;
	FAGX_ConstraintRangeController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier);
	void CopyFrom(const FRangeControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};
