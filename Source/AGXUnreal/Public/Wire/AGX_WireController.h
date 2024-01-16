// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireControllerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_WireController.generated.h"

class UAGX_WireComponent;

UCLASS(Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_WireController : public UObject
{
	GENERATED_BODY()

public:
	UAGX_WireController();

	UFUNCTION(BlueprintCallable, Category = "Wire Controller", Meta = (DisplayName = "Get Wire Controller"))
	static UAGX_WireController* Get();

	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool IsWireWireActive() const;

	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool SetCollisionsEnabled(UAGX_WireComponent* Wire1, UAGX_WireComponent* Wire2, bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool GetCollisionsEnabled(const UAGX_WireComponent* Wire1, const UAGX_WireComponent* Wire2) const;

	bool HasNative() const;

private:
	FWireControllerBarrier NativeBarrier;
};
