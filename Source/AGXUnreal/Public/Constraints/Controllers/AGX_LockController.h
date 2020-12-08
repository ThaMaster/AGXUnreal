#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

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
	UPROPERTY(
		EditAnywhere, Category = "AGX Lock Controller", Meta = (EditCondition = "bEnable"))
	 double Position;

public:
	FAGX_ConstraintLockController() = default;
	FAGX_ConstraintLockController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FLockControllerBarrier> Barrier);
	void CopyFrom(const FLockControllerBarrier& Source);

protected:
	virtual void UpdateNativePropertiesImpl() override;
};


/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintLockController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
