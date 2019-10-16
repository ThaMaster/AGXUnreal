#pragma once


namespace agx
{
	class RangeController;
}

/**
 * Used to push a block of data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * the conversion in one common place.
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