#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ScrewController.generated.h"

class FScrewControllerBarrier;

/**
 * Screw controller that puts a relationship between two free DOFs of a
 * constraint, given that one free DOF is translational and the other free DOF
 * is rotational. Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintScrewController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()


	/**
	 * The distance, in centimeters along the screw's axis, that is covered by
	 * one complete rotation of the screw (360 degrees).
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Lead;

public:
	FAGX_ConstraintScrewController() = default;
	FAGX_ConstraintScrewController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FScrewControllerBarrier> Barrier);
	void CopyFrom(const FScrewControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};
