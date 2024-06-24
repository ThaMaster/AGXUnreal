// Copyright 2024, Algoryx Simulation AB.

#pragma once

#if 1

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ElementaryConstraint.h"
#include "Constraints/TwistRangeControllerBarrier.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_TwistRangeController.generated.h"

/**
 * A Twist Range Controller limits the range of relative rotation between two Rigid Bodies. It is
 * only available on Constraint Components that allow for multiple axes of rotation of which one
 * is the twist axis. Ball Constraint is an example of such a Constraint Component. Hinge is not
 * since it only allows for rotation around a single axis and therefore has a Range Controller
 * instead.
 *
 * Twist is defined to be rotation around the constraint's Z axis.
 *
 * This type cannot be used to create new native AGX Dynamics instances of the Twist Range
 * Controller, it is only used to access a Twist Range Controller that exists on a constraint such
 * as Ball Constraint.
 *
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TwistRangeController : public FAGX_ElementaryConstraint
{
	GENERATED_BODY()

public: // Special member functions.
	FAGX_TwistRangeController();
	virtual ~FAGX_TwistRangeController();

public: // Properties.
	/**
	 * The amount of rotation around the constraint's Z axis that is allowed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Twist Range Controller", Meta = (EditCondition = "bEnabled"))
	FAGX_RealInterval Range;

	void SetRange(double InRangeMin, double InRangeMax);
	void SetRange(FAGX_RealInterval InRange);
	double GetRangeMin() const;
	double GetRangeMax() const;

	virtual void UpdateNativeProperties() override;

public: // Native management.
	bool HasNative() const;
	FTwistRangeControllerBarrier* GetNative();
	const FTwistRangeControllerBarrier* GetNative() const;

	/**
	 * Bind this Twist Range Controller to the AGX Dynamics Twist Range Controller referenced by
	 * the given Barrier.
	 * @param Barrier
	 */
	void InitializeBarrier(const FTwistRangeControllerBarrier& InBarrier);

	/**
	 * Copy property values from the given Source Barrier into this.
	 *
	 * @param Source The Barrier to read through.
	 * @param ArchetypeInstances Template instances to update.
	 * @param bForceOverwriteInstances Whether to also update instances with edited values.
	 */
	void CopyFrom(
		const FTwistRangeControllerBarrier& Source,
		TArray<FAGX_TwistRangeController*>& ArchetypeInstances, bool bForceOverwriteInstances);

private:
	FTwistRangeControllerBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_TwistRangeController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_TwistRangeController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Twist Range Controller")
	static void SetRange(UPARAM(Ref) FAGX_TwistRangeController& Controller, double Min, double Max)
	{
		Controller.SetRange(Min, Max);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Twist Range Controller")
	static void GetRange(const FAGX_TwistRangeController& ControllerRef, double& Min, double& Max)
	{
		Min = ControllerRef.GetRangeMin();
		Max = ControllerRef.GetRangeMax();
	}

	// See comment in AGX_ElementaryConstraint.h.

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_TwistRangeController& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetCompliance(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Compliance)
	{
		return Constraint.SetCompliance(Compliance);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetCompliance(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetCompliance();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetSpookDamping(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetSpookDamping();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRange(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Max)
	{
		Constraint.SetForceRangeMin(Max);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void GetForceRange(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double& Min, double& Max)
	{
		FDoubleInterval ForceRange = Constraint.GetForceRange();
		Min = ForceRange.Min;
		Max = ForceRange.Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMin(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
};

#endif
