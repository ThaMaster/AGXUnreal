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

private:
	FContactPointBarrier Barrier;
};
