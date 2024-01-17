// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireControllerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_WireController.generated.h"

class UAGX_WireComponent;

/**
 * The Wire Controller handles inter-wire and global wire settings such as wire-wire collision
 * detection and dynamic wire contacts.
 *
 * On the AGX Dynamics side Wire Controller is a singleton.
 */
UCLASS(Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_WireController : public UObject
{
	GENERATED_BODY()

public:
	UAGX_WireController();

	/**
	 * Get the Wire Controller.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "Wire Controller",
		Meta = (DisplayName = "Get Wire Controller"))
	static UPARAM(DisplayName = "Wire Controller") UAGX_WireController* Get();

	/**
	 * Returns true if collision detection is enabled for at least one wire-wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool IsWireWireActive() const;

	/**
	 * Enable or disable collision detection for the given wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool SetCollisionsEnabled(UAGX_WireComponent* Wire1, UAGX_WireComponent* Wire2, bool bEnable);

	/**
	 * Returns true if collision detection is enabled for the given wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "Wire Controller")
	bool GetCollisionsEnabled(const UAGX_WireComponent* Wire1, const UAGX_WireComponent* Wire2) const;

	/**
	 * Returns true if this Wire Controller has a Native, which it always should have.
	 */
	bool HasNative() const;

private:
	FWireControllerBarrier NativeBarrier;
};
