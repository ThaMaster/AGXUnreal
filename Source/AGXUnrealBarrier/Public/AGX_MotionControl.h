#pragma once

#include "UObject/ObjectMacros.h"

UENUM()
enum EAGX_MotionControl
{
	/** Rigid body will never move. */
	MC_STATIC = 1 UMETA(DisplayName = "Static"),

	/** Rigid body's motion is "scripted" (i.e. set by user or some Unreal system). */
	MC_KINEMATICS = 2 UMETA(DisplayName = "Kinematics"),

	/** Rigid body moves from the influence of AGX simulation forces. */
	MC_DYNAMICS = 3 UMETA(DisplayName = "Dynamics"),
};
