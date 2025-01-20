// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireParameterControllerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireParameterController.generated.h"

class FWireParameterControllerBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireParameterController
{
	GENERATED_BODY()

	void SetBarrier(const FWireParameterControllerBarrier& InBarrier);

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double MaximumContactMovementOneTimestep {0.1};
	void SetMaximumContactMovementOneTimestep(double MaxMovement);
	double GetMaximumContactMovementOneTimestep() const;

	/**
	 * Set the minimum distance allowed between nodes. I.e., a lumped element node will NOT
	 * be created closer than this distance from routed nodes.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double MinimumDistanceBetweenNodes {5.0};
	void SetMinimumDistanceBetweenNodes(double MinDistance);
	double GetMinimumDistanceBetweenNodes() const;

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire. The parameter
	 * has an analytical value derived given the Nyquist frequency. The probability to have more
	 * lumped nodes in the wire increases with this scale constant.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double ScaleConstant {0.35};
	void SetScaleConstant(double InScaleConstant);
	double GetScaleConstant() const;

	void WritePropertiesToNative();
	bool HasNative() const;

private:
	FWireParameterControllerBarrier NativeBarrier;
};

UCLASS()
class AGXUNREAL_API UAGX_WireParameterController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetMaximumContactMovementOneTimestep(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double MaxMovement)
	{
		Controller.SetMaximumContactMovementOneTimestep(MaxMovement);
	}

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetMaximumContactMovementOneTimeStep(UPARAM(Ref)
														   FAGX_WireParameterController& Controller)
	{
		return Controller.GetMaximumContactMovementOneTimestep();
	}

	/**
	 * Set the minimum distance allowed between nodes. I.e., a lumped element node will NOT
	 * be created closer than this distance from routed nodes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetMinimumDistanceBetweenNodes(UPARAM(Ref) FAGX_WireParameterController& Controller, double MinDistance)
	{
		Controller.SetMinimumDistanceBetweenNodes(MinDistance);
	}

	/**
	 * Set the minimum distance allowed between nodes. I.e., a lumped element node will NOT
	 * be created closer than this distance from routed nodes.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetMinimumDistanceBetweenNodes(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetMinimumDistanceBetweenNodes();
	}

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire. The parameter
	 * has an analytical value derived given the Nyquist frequency. The probability to have more
	 * lumped nodes in the wire increases with this scale constant.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetScaleConstant(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double ScaleConstant)
	{
		Controller.SetScaleConstant(ScaleConstant);
	}

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire. The parameter
	 * has an analytical value derived given the Nyquist frequency. The probability to have more
	 * lumped nodes in the wire increases with this scale constant.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetScaleConstant(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetScaleConstant();
	}
};
