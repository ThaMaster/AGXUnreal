// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternBase.h"

#include "AGX_RayPatternHorizontalSweep.generated.h"

/**
 * Scans one vertical line, then goes to the next.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_RayPatternHorizontalSweep : public UAGX_RayPatternBase
{
	GENERATED_BODY()
};
