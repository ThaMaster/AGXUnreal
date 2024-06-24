// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ElementaryConstraintBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Interval.h"

#include "AGX_ElementaryConstraint.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ElementaryConstraint
{
	GENERATED_BODY()

public:
	FAGX_ElementaryConstraint();
	virtual ~FAGX_ElementaryConstraint();

	/**
	 * Whether this Constraint is enabled or not. A disabled Constraint has no effect on the
	 * simulation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Controller")
	bool bEnable {true};

	void SetEnable(bool bInEnable);

	bool GetEnable() const;

	/**
	 * The compliance in a certain DOF. Measured in [m/N] for translational DOFs and [rad/Nm] for
	 * rotational DOFs.
	 *
	 * The compliance measure the inverse of stiffness of the Constraint Controller. A smaller
	 * compliance will cause a stronger force or torque to be created to restore from the violation.
	 * A too small compliance will lead to instabilities in the simulation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller",
		Meta =
			(EditCondition = "bEnable", SliderMin = "1e-10", SliderMax = "1",
			 SliderExponent = "100000"))
	FAGX_Real Compliance {ConstraintConstants::DefaultCompliance()};

	void SetCompliance(double InCompliance);

	double GetCompliance() const;

	/**
	 * Elasticity is defined as the inverse of compliance, so setting the elasticity will modify
	 * the compliance and vice versa [N/m] or [Nm/rad].
	 */
	void SetElasticity(double InElasticity);

	double GetElasticity() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FAGX_Real SpookDamping {ConstraintConstants::DefaultSpookDamping()};

	void SetSpookDamping(double InSpookDamping);

	double GetSpookDamping() const;

	/**
	 * The minimum and maximum force or torque that the constraint controller can produce [N] or
	 * [Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval ForceRange {ConstraintConstants::DefaultForceRange()};

	void SetForceRange(const FDoubleInterval& InForceRange);
	void SetForceRange(const FAGX_RealInterval& InForceRange);
	void SetForceRange(double InMinForce, double InMaxForce);
	void SetForceRangeMin(double InMinForce);
	void SetForceRangeMax(double InMaxForce);

	FDoubleInterval GetForceRange() const;
	double GetForceRangeMin() const;
	double GetForceRangeMax() const;

	/**
	 * Get the force or torque that was applied by this Elementary constraint in the most recent
	 * time step.
	 */
	double GetForce();

	/**
	 * Whether the constraint was active or not during the most recent time step. Most constraints
	 * are active as long as they are enabled but some activate based on simulation state, such as
	 * Range Controller activating when the range is hit.
	 */
	bool IsActive() const;

	/**
	 * Get the AGX Dynamics internal name for this constraint.
	 */
	FString GetName() const;

	bool HasNative() const;
	FElementaryConstraintBarrier* GetNative();
	const FElementaryConstraintBarrier* GetNative() const;

	void InitializeBarrier(const FElementaryConstraintBarrier& InBarrier);

	/**
	 * Apply the UProperties on the native AGX Dynamics constraint. May only be called if HasNative
	 * returns true.
	 */
	virtual void UpdateNativeProperties();

	/**
	 * Copy properties from the give AGX Dynamics constraint controller into this AGXUnreal
	 * constraint controller.
	 */
	void CopyFrom(
		const FElementaryConstraintBarrier& Source,
		TArray<FAGX_ElementaryConstraint*>& ArchetypeInstances, bool bForceOverwriteInstances);

protected:
	FElementaryConstraintBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_ElementaryConstraint in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ElementaryConstraint_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetCompliance(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Compliance)
	{
		return Constraint.SetCompliance(Compliance);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetCompliance(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetCompliance();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetSpookDamping(
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetSpookDamping();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRange(
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Max)
	{
		Constraint.SetForceRangeMin(Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void GetForceRange(
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double& Min, double& Max)
	{
		FDoubleInterval ForceRange = Constraint.GetForceRange();
		Min = ForceRange.Min;
		Max = ForceRange.Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMin(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
};

// We have substructs of FAGX_ElementaryConstraint which we also want to have the base struct
// Blueprint Library functions. Unfortunately, we have not found a way to automate this yet. The
// Blueprint Library declared above isn't callable on the subtypes and a #define containing
// all the function declarations and definitions doesn't work because UHT doesn't expand macros. So
// for now we're stuck with copy/paste. The following code block should be copy/pasted in each
// Elementary Constraint sublass' Blueprint Library class. Search/replace FAGX_TYPE and substitute
// the actual Elementary Constraint subtype.
/*
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_TYPE& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetCompliance(UPARAM(Ref) FAGX_TYPE& Constraint, double Compliance)
	{
		return Constraint.SetCompliance(Compliance);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetCompliance(UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetCompliance();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetSpookDamping(
		UPARAM(Ref) FAGX_TYPE& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetSpookDamping();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRange(
		UPARAM(Ref) FAGX_TYPE& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_TYPE& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_TYPE& Constraint, double Max)
	{
		Constraint.SetForceRangeMin(Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void GetForceRange(
		UPARAM(Ref) FAGX_TYPE& Constraint, double& Min, double& Max)
	{
		FDoubleInterval ForceRange = Constraint.GetForceRange();
		Min = ForceRange.Min;
		Max = ForceRange.Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMin(
		UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(
		UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
*/
