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

	void SetEnable(bool bInEnable);

	bool GetEnable() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	double Compliance;

	void SetCompliance(double InCompliance);

	double GetCompliance() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	double Damping;

	void SetDamping(double InDamping);

	double GetDamping() const;

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

	void SetForceRange(const FFloatInterval& InForceRange);

	FFloatInterval GetForceRange() const;

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
	const FConstraintControllerBarrier* GetNative() const;

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
	static void SetEnable(UPARAM(ref) FAGX_ConstraintController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(UPARAM(ref) FAGX_ConstraintController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetDamping(UPARAM(ref) FAGX_ConstraintController& Controller, float Damping)
	{
		Controller.SetDamping(static_cast<double>(Damping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetDamping(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return static_cast<float>(Controller.GetDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(FFloatInterval(MinForce, MaxForce));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return Controller.GetForceRange().Max;
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
	static void SetEnable(UPARAM(ref) FAGX_TYPE& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(UPARAM(ref) FAGX_TYPE& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetDamping(UPARAM(ref) FAGX_TYPE& Controller, float Damping)
	{
		Controller.SetDamping(static_cast<double>(Damping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetDamping(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return static_cast<float>(Controller.GetDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_TYPE& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(FFloatInterval(MinForce, MaxForce));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}
*/
