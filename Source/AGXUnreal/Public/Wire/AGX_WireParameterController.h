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

	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double ScaleConstant {0.35};

	void SetScaleConstant(double InScaleConstant);
	double GetScaleConstant() const;

	void WritePropertiesToNative();

	bool HasNative() const;

private:
	FWireParameterControllerBarrier Barrier;
};

UCLASS()
class AGXUNREAL_API UAGX_WireParameterController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetScaleConstant(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double ScaleConstant)
	{
		Controller.SetScaleConstant(ScaleConstant);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetScaleConstant(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetScaleConstant();
	}
};
