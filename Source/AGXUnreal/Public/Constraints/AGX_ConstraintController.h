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

	double GetForce();

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

	// We must provide an assignment operator because Unreal must be able to copy structs and we
	// contain a non-copyable TUniquePtr to the underlying Barrier object. That object is not
	// copied.
	/**
	 * Copy the properties from Other into this. The underlying Barrier object will neither be
	 * copied nor shared.
	 * @param Other The object to copy parameters from.
	 * @return A reference to this.
	 */
	FAGX_ConstraintController& operator=(const FAGX_ConstraintController& Other);

	bool HasNative() const;
	FConstraintControllerBarrier* GetNative();

	void UpdateNativeProperties();

protected:
	virtual void UpdateNativePropertiesImpl()
		PURE_VIRTUAL(FAGX_ConstraintController::UpdateNativePropertiesImpl, );

	/**
	 * Copy properties from the give AGX Dynamics constraint controller into this AGXUnreal
	 * constraint controller.
	 */
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

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}
};

// We have substructs of FAGX_ConstraintController which we also want to have the base struct
// Blueprint Library functions. Unfortunately, we have not found a way to automate this yet. The
// Blueprint Library declared above isn't callable on the subtypes and a #define containing
// all the function declarations and definitions doesn't work because UHT doesn't expand macros. So
// for now we're stuck with copy/paste. The following code block should be copy/pasted in each
// ConstraintController Blueprint Library class. Search/replace FAGX_TYPE and substitute the actual
// ConstraintController subtype.
/*
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}
*/
