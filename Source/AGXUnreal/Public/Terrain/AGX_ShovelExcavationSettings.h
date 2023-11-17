// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShovelExcavationSettings.generated.h"

USTRUCT(BlueprintType)
struct FAGX_ShovelExcavationSettings
{
	GENERATED_BODY()

	/**
	 * Whether this excavation mode should be creating dynamic mass and generating force feedback or
	 * not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnabled {true};

	/**
	 * Whether the excavation mode should create dynamic mass or not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnableCreateDynamicMass {true};

	/**
	 * Whether the excavation mode should generate force feedback from created aggregates or not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnableForceFeedback {true};
};
