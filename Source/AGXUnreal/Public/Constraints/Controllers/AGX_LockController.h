#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LockController.generated.h"

class FLockControllerBarrier;

/**
 * Lock controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintLockController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Target position in Degrees if controller is on a Rotational
	 * Degree-Of-Freedom, else in Centimeters.
	 */
	 UPROPERTY(EditAnywhere, Meta =	(EditCondition = "bEnable"))
	 double Position;

public:
	FAGX_ConstraintLockController() = default;
	FAGX_ConstraintLockController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FLockControllerBarrier> Barrier);
	void CopyFrom(const FLockControllerBarrier& Source);

protected:
	virtual void UpdateNativePropertiesImpl() override;
};
