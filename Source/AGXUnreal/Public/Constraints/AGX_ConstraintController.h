#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Interval.h"

#include "AGX_ConstraintController.generated.h"

class FConstraintControllerBarrier;

/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Controller")
	bool bEnable;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	double Compliance;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	double Damping;

	/// \todo Should this be in N (Nm) or some cm-based unit?
	/// Cannot be TInterval<double> because that type is not listed in
	/// Math/Interval.h
	/**
	 * The minimum an maximum force that the constraint controller can produce.
	 * In newtons, i.e., kg m s^-2.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

protected:
	// Would like to have this const but Unreal provides default copy operations
	// that don't compile when USTRUCT structs contains constant members.
	// Perhaps there is a way to disable the default copy operations.
	/// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

public:
	FAGX_ConstraintController();
	FAGX_ConstraintController(bool bInRotational);
	virtual ~FAGX_ConstraintController();
	FAGX_ConstraintController& operator=(const FAGX_ConstraintController& Other);
	/// \todo Consider adding operator= to derived classes.

	bool HasNative() const;
	FConstraintControllerBarrier* GetNative();

	void UpdateNativeProperties();

protected:
	virtual void UpdateNativePropertiesImpl()
		PURE_VIRTUAL(FAGX_ConstraintController::UpdateNativePropertiesImpl, );
	void CopyFrom(const FConstraintControllerBarrier& Source);

	TUniquePtr<FConstraintControllerBarrier> NativeBarrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_ConstraintController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}
};
