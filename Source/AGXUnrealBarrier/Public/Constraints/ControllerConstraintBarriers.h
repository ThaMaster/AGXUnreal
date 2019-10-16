#pragma once


namespace agx
{
	class ElectricMotorController;
	class FrictionController;
	class LockController;
	class RangeController;
	class ScrewController;
	class TargetSpeedController;
}


/**
 * Used to transfer Electric Motor Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FElectricMotorControllerBarrier
{
	bool bEnable;
	double ForceRangeMin;
	double ForceRangeMax;

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

	double Voltage;
	double ArmatureResistance;
	double TorqueConstant;

	void ToNative(agx::ElectricMotorController* Native, UWorld* World) const;
};


/**
 * Used to transfer Friction Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FFrictionControllerBarrier
{
	bool bEnable;
	double Elasticity;
	double Damping;
	double ForceRangeMin;
	double ForceRangeMax;

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

	double FrictionCoefficient;
	bool bEnableNonLinearDirectSolveUpdate;

	void ToNative(agx::FrictionController* Native, UWorld* World) const;
};


/**
 * Used to transfer Lock Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FLockControllerBarrier
{
	bool bEnable;
	double Elasticity;
	double Damping;
	double ForceRangeMin;
	double ForceRangeMax;

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

	double Position;

	void ToNative(agx::LockController* Native, UWorld* World) const;
};


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
 * Used to transfer Screw Controller data between Unreal and AGX natives,
 * instead of having functions with many variables. Also handles
 * unit conversions in one common place.
 */
struct FScrewControllerBarrier
{
	bool bEnable;
	double Elasticity;
	double Damping;
	double ForceRangeMin;
	double ForceRangeMax;

	double Lead;

	void ToNative(agx::ScrewController* Native, UWorld* World) const;
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