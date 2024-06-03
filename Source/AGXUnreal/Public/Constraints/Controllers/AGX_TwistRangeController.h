// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ElementaryConstraint.h"
#include "Constraints/ControllerConstraintBarriers.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Interval.h"

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

public:
	FAGX_TwistRangeController();
	virtual ~FAGX_TwistRangeController();

	/**
	 * The amount of rotation around the constraint's Z axis that is allowed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Twist Range Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval Range;

	/**
	 * Set the amount of rotation around the constraint's Z axis that is allowed.
	 */
	void SetRange(FDoubleInterval InRange);

	/**
	 * Set the amount of rotation around the constraint's Z axis that is allowed.
	 */
	void SetRange(FAGX_RealInterval InRange);

	void SetRangeMin(double InMin);
	void SetRangeMax(double InMax);

	/**
	 * Get the amount of rotation around the constraint's Z axis that is allowed.
	 */
	FDoubleInterval GetRange() const;
	double GetRangeMin() const;
	double GetRangeMax() const;

	bool HasNative() const;
	FTwistRangeControllerBarrier* GetNative();
	const FTwistRangeControllerBarrier* GetNative() const;

	/**
	 * Bind this Twist Range Controller
	 * @param Barrier
	 */
	void InitializeBarrier(const FTwistRangeControllerBarrier& Barrier);

	void UpdateNativeProperties();

	void CopyFrom(
		const FTwistRangeControllerBarrier& Source,
		TArray<FAGX_TwistRangeController*>& ArchetypeInstances, bool bForceOverwriteInstances);

private:
	FTwistRangeControllerBarrier NativeBarrier;
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
		Controller.SetRange(FDoubleInterval(Min, Max));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Twist Range Controller")
	static void SetRangeMin(UPARAM(Ref) FAGX_TwistRangeController& Controller, double Min)
	{
		Controller.SetRangeMin(Min);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Twist Range Controller")
	static void SetRangeMax(UPARAM(Ref) FAGX_TwistRangeController& Controller, double Max)
	{
		Controller.SetRangeMax(Max);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Twist Range Controller")
	static void GetRange(
		const FAGX_TwistRangeController& ControllerRef, double& OutMin, double& OutMax)
	{
		FDoubleInterval Range = ControllerRef.GetRange();
		OutMin = Range.Min;
		OutMax = Range.Max;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Twist Range Controller")
	static double GetRangeMin(const FAGX_TwistRangeController& ControllerRef)
	{
		return ControllerRef.GetRangeMin();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Twist Range Controller")
	static double GetRangeMax(const FAGX_TwistRangeController& ControllerRef)
	{
		return ControllerRef.GetRangeMax();
	}

	// TODO Add the rest of the functions.
};
