#pragma once

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

struct FConstraintControllerRef;

class AGXUNREALBARRIER_API FConstraintControllerBarrier
{
public:
	// FConstraintControllerBarrier(); /// \todo Will this be needed?
	FConstraintControllerBarrier(FConstraintControllerBarrier&& Other) noexcept;
	FConstraintControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);
	virtual ~FConstraintControllerBarrier(); /// \todo Can this be removed?
	FConstraintControllerBarrier& operator=(FConstraintControllerBarrier&& Other) noexcept;

	bool HasNative() const;
	FConstraintControllerRef* GetNative();
	const FConstraintControllerRef* GetNative() const;

	void SetCompliance(float Compliance);
	float GetCompliance() const;

	void SetDamping(float Damping);
	float GetDamping() const;

	void SetForceRange(FFloatInterval ForceRange);
	FFloatInterval GetForceRange() const;

private:
	FConstraintControllerBarrier(const FConstraintControllerBarrier&) = delete;
	void operator=(const FConstraintControllerBarrier&) = delete;

protected:
	std::unique_ptr<FConstraintControllerRef> NativeRef;
};

class AGXUNREALBARRIER_API FElectricMotorControllerBarrier : public FConstraintControllerBarrier
{
public:
	//FElectricMotorControllerBarrier(); Will this be needed?
	FElectricMotorControllerBarrier(FElectricMotorControllerBarrier&& Other) noexcept;
	FElectricMotorControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);
	virtual ~FElectricMotorControllerBarrier();
	FElectricMotorControllerBarrier& operator=(FElectricMotorControllerBarrier&& Other) noexcept;

	void SetVoltage(float Voltage);
	float GetVoltage() const;

	void SetArmatureResistance(float ArmatureResistance);
	float GetArmatureResistance() const;

	void SetTorqueConstant(float TorqueConstant);
	float GetTorqueConstant() const;

private:
};

#if 0
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

	void ToNative(agx::ElectricMotorController* Native) const;
	void FromNative(const agx::ElectricMotorController& Native);
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

	void ToNative(agx::FrictionController* Native) const;
	void FromNative(const agx::FrictionController& Native);
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

	void ToNative(agx::LockController* Native) const;
	void FromNative(const agx::LockController& Native);
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

	void ToNative(agx::RangeController* Native) const;
	void FromNative(const agx::RangeController& Native);
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

	void ToNative(agx::ScrewController* Native) const;
	void FromNative(const agx::ScrewController& Native);
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

	void ToNative(agx::TargetSpeedController* Native) const;
	void FromNative(const agx::TargetSpeedController& Native);
};
#endif
