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

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnabled(UPARAM(Ref) FAGX_TwistRangeController& Constraint, bool Enable)
	{
		return Constraint.SetEnabled(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnabled(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetEnabled();
	}

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
};

#endif
