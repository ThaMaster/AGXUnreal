#pragma once


namespace agx
{
	class RangeController;
	class TargetSpeedController;
}

/**
 * Used to transfer Range Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FRangeControllerBarrier
{
	bool bEnable;
	double Elasticity;
	double Damping;
	double ForceRangeMin;
	double ForceRangeMax;

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

	// Radians if bRotational is true, else Centimeters.
	double RangeMin;
	double RangeMax;

	void ToNative(agx::RangeController* Native, UWorld* World) const;
};


/**
 * Used to transfer Target Speed Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FTargetSpeedControllerBarrier
{
	bool bEnable;
	double Elasticity;
	double Damping;
	double ForceRangeMin;
	double ForceRangeMax;

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

	// Radians per sec if bRotational is true, else Centimeters per sec.
	double Speed;

	bool bLockedAtZeroSpeed;

	void ToNative(agx::TargetSpeedController* Native, UWorld* World) const;
};