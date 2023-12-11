// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * Enum identifying a free degree of freedom in a 2-DOF constraint.
 */
UENUM()
enum class EAGX_Constraint2DOFFreeDOF : uint8
{
	FIRST UMETA(DisplayName = "First"),
	SECOND UMETA(DisplayName = "Second")
};
