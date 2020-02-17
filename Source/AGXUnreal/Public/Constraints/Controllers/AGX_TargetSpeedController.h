#pragma once

// AGXUnreal includes.
#include "AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TargetSpeedController.generated.h"

class FTargetSpeedControllerBarrier;

/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint). Disabled by
 * default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintTargetSpeedController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Target Speed in Degrees Per Second if controller is on a Rotational DOF,
	 * else in Centimeters Per Second.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Target Speed Controller", Meta = (EditCondition = "bEnable"))
	double Speed;

	/**
	 * Whether the controller should auto-lock whenever target speed is zero,
	 * such that it will not drift away from that angle/position if the assigned
	 * force range is enough to hold it.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Target Speed Controller", Meta = (EditCondition = "bEnable"))
	bool bLockedAtZeroSpeed;

public:
	FAGX_ConstraintTargetSpeedController() = default;
	FAGX_ConstraintTargetSpeedController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FTargetSpeedControllerBarrier> Barrier);
	void CopyFrom(const FTargetSpeedControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};
