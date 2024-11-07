// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_BrickInputs.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Input
{
	GENERATED_BODY()

	FPLX_Input() = default;
	explicit FPLX_Input(const FString& InName)
		: Name(InName)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PLX")
	FString Name;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_LinearVelocityMotorVelocityInput : public FPLX_Input
{
	GENERATED_BODY()

	FPLX_LinearVelocityMotorVelocityInput() = default;
	explicit FPLX_LinearVelocityMotorVelocityInput(const FString& InName)
		: FPLX_Input(InName)
	{
	}
};
