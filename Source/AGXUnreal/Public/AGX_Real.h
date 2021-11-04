#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Real.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_Real
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	double Value = 0.0;

	FAGX_Real()
	{
	}

	FAGX_Real(double InValue)
		: Value(InValue)
	{
	}

	operator double() const
	{
		return Value;
	}

	operator double&()
	{
		return Value;
	}

	FAGX_Real& operator=(double InValue)
	{
		Value = InValue;
		return *this;
	}
};
