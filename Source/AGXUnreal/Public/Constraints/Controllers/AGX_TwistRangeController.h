// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/ControllerConstraintBarriers.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
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
 */
/*
Unfortunately we cannot inherit from FAGX_ConstraintController here because the underlying AGX
Dynamics type doesn't inherit from the corresponding BasicControllerConstraint.
*/
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TwistRangeController
{
	GENERATED_BODY()

public:
	FAGX_TwistRangeController();
	virtual ~FAGX_TwistRangeController();

	/**
	 * Whether this Constraint Controller is active or not. A disabled Constraint Controller has
	 * no effect on the simulation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Controller")
	bool bEnable {false};

	void SetEnable(bool bInEnable);

	bool GetEnable() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Twist Range Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval Range;

	void SetRange(FAGX_RealInterval InRange);
	FAGX_RealInterval GetRange() const;

	// TODO How handle compliance, damping, and all that? Another base class?

	// We must provide an assignment operator because Unreal must be able to copy structs and we
	// contain a non-copyable TUniquePtr to the underlying Barrier object. That object is not
	// copied.

	// TODO Consider what the assignment operator should do. We no longer carry a unique pointer
	// and the Barrier type could be made assignable. What do we want to happen? Will this code
	// run when accessing the Twist Range Controller from a Blueprint Visual Script?

	/**
	 * Copy the properties from Other into this. The underlying Barrier object will neither be
	 * copied nor shared.
	 * @param Other The object to copy parameters from.
	 * @return A reference to this.
	 */
	FAGX_TwistRangeController& operator=(const FAGX_TwistRangeController& Other);

	void InitializeBarrier(const FTwistRangeControllerBarrier& Barrier);
	void CopyFrom(
		const FTwistRangeControllerBarrier& Source,
		TArray<FAGX_TwistRangeController*>& ArchetypeInstances, bool bForceOverwriteInstances);

	bool HasNative() const;
	FTwistRangeControllerBarrier* GetNative();
	const FTwistRangeControllerBarrier* GetNative() const;

	void UpdateNativeProperties();

private:
	FTwistRangeControllerBarrier NativeBarrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_TwistRangeController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_TwistRangeController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_TwistRangeController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	// TODO Add the rest of the functions.
};
