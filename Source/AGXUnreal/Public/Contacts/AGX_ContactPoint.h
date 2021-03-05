#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ContactPoint.generated.h"

USTRUCT(Category = "AGX", BlueprintType)
struct AGXUNREAL_API FAGX_ContactPoint
{
	GENERATED_BODY()

public:

	FAGX_ContactPoint() = default;
	FAGX_ContactPoint(const FAGX_ContactPoint& InOther);
	FAGX_ContactPoint(FContactPointBarrier&& InBarrier);
	FAGX_ContactPoint& operator=(const FAGX_ContactPoint& InOther);

	bool HasNative() const;

	bool IsEnabled() const;

	float GetDepth() const;
	FVector GetLocation() const;
	FVector GetNormal() const;
	FVector GetTangentU() const;
	FVector GetTangentV() const;

	FVector GetForce() const;
	FVector GetNormalForce() const;
	FVector GetTangentialForce() const;
	FVector GetLocalForce() const;

	FVector GetVelocity() const;
	FVector GetWitnessPoint(int32 Index) const;
	float GetArea() const;

private:
	FContactPointBarrier Barrier;
};
