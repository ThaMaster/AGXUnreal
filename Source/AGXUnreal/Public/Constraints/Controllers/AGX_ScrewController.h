#pragma once

#include "CoreMinimal.h"

#include "AGX_ScrewController.generated.h"

/**
 * Screw controller that puts a relationship between two free DOFs of a constraint,
 * given that one free DOF is translational and the other free DOF is rotational.
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintScrewController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller")
	bool bEnable;

	/**
	 * The distance, in centimeters along the screw's axis, that is covered by
	 * one complete rotation of the screw (360 degrees).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	double Lead;

	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:
	FAGX_ConstraintScrewController(bool bRotational = false);

	void ToBarrier(struct FScrewControllerBarrier* Barrier) const;
};
