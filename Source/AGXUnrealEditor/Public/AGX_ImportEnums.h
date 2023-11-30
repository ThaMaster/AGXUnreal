// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

/** Specifies what type of import is being performed. */
UENUM()
enum class EAGX_ImportType : uint8
{
	/** Imported type is Invalid. */
	Invalid,

	/** Imported type is AGX Dynamics Archive. */
	Agx,

	/** Imported type is URDF (Unified Robotic Description Format) model. */
	Urdf
};
