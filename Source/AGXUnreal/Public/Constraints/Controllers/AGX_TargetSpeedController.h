#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_TargetSpeedController.generated.h"

class FTargetSpeedControllerBarrier;

/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint). Disabled by
 * default.
 */
USTRUCT(BlueprintType)
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

	void SetSpeed(double InSpeed);
	double GetSpeed() const;

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintTargetSpeedController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static void SetSpeed(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& ControllerRef, const float Speed)
	{
		ControllerRef.SetSpeed(static_cast<double>(Speed));
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static float GetSpeed(UPARAM(ref) FAGX_ConstraintTargetSpeedController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetSpeed());
	};

	//~ Functions inherited from AGX_ConstraintController.
	/// \todo Why can't the functions in UAGX_ConstraintController_FL be called on a
	/// FAGX_ConstraintTargetSpeedController?
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintTargetSpeedController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}
};
