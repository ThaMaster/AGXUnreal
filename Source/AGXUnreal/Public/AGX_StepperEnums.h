#pragma once

// Unreal Engine includes.
#include "UObject/ObjectMacros.h"

UENUM()
enum EAGX_StepMode
{
	/** Step the AGX simulation as many times as necessary per Unreal step to keep Unreal and AGX time synchronized at all time. May result in low framerate. */
	SM_CATCH_UP_IMMEDIATELY UMETA(DisplayName = "Catch up immediately"),

	/** Allow up to two AGX simulation steps per Unreal step whenever the AGX simulation lags behind. May result in some stuttering. */
	SM_CATCH_UP_OVER_TIME UMETA(DisplayName = "Catch up over time"),

	/** Similar to 'Catch up over time' but will behave as 'Drop immediately' if the AGX simulation lags behind more then the specified Time Lag Cap. */
	SM_CATCH_UP_OVER_TIME_CAPPED UMETA(DisplayName = "Catch up over time Capped"),

	/** Step the AGX simulation exactly one time per Unreal step. May result in simulation appearing to run in slow-motion. */
	SM_DROP_IMMEDIATELY UMETA(DisplayName = "Drop immediately"),
};
