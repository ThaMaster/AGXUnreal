// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternBase.h"

#include "AGX_RayPatternCustom.generated.h"

/**
 * Use a custom scan pattern. Ray transforms are set by the user.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_RayPatternCustom : public UAGX_RayPatternBase
{
	GENERATED_BODY()
};
