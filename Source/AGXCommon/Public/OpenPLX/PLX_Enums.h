// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum class EPLX_InputType
{
	Unsupported,
	LinearVelocity1DInput
};

UENUM()
enum class EPLX_OutputType
{
	Unsupported,
	AngleOutput
};
