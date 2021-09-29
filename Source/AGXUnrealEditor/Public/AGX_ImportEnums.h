#pragma once

#include "CoreMinimal.h"

/** Specifies what type of import is being performed. */
UENUM()
enum EAGX_ImportType
{
	/** Imported type none. */
	Invalid,

	/** Imported type is AGX Dynamics Archive. */
	Agx,

	/** Imported type is URDF (Unified Robotic Description Format) model. */
	Urdf
};
