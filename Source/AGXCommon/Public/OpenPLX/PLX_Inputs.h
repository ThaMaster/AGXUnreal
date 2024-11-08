// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "PLX_Inputs.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Input
{
	GENERATED_BODY()

	FPLX_Input() = default;
	explicit FPLX_Input(const FString& InName)
		: Name(InName)
	{
	}

	virtual ~FPLX_Input() = default;

	UPROPERTY()
	FString Name;

	virtual UScriptStruct* GetType() const
	{
		return FPLX_Input::StaticStruct();
	}
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

	virtual UScriptStruct* GetType() const override
	{
		return FPLX_LinearVelocityMotorVelocityInput::StaticStruct();
	}
};
