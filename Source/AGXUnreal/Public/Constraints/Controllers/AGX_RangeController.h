#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

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
	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	FFloatInterval Range;

public:
	FAGX_ConstraintRangeController() = default;
	FAGX_ConstraintRangeController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier);
	void CopyFrom(const FRangeControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintRangeController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
