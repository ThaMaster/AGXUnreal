// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREAL_API FAGX_ImporterUtilities
{
public:
	static void CopyProperties(
		const UObject& Source, UObject& OutDestination, bool UpdateArchetypeInstances = true);
};
