// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShovelExcavationSettings.generated.h"


USTRUCT()
struct FAGX_ShovelExcavationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Shovel Excavation Settings")
	bool bEnabled {true};

	UPROPERTY(EditAnywhere, Category = "AGX Shovel Excavation Settings")
	bool bEnableCreateDynamicMass {true};

	UPROPERTY(EditAnywhere, Category = "AGX Shovel Excavation Settings")
	bool bEnableForceFeedback {true};
};
