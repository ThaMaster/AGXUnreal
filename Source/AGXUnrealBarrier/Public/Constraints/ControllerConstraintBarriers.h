// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

struct FConstraintControllerRef;
struct FTwistRangeControllerRef;

class AGXUNREALBARRIER_API FConstraintControllerBarrier
{
public:
	virtual ~FConstraintControllerBarrier();

	bool HasNative() const;
	FConstraintControllerRef* GetNative();
	const FConstraintControllerRef* GetNative() const;

	void SetEnable(bool bEnabled);
	bool GetEnable() const;

	void SetCompliance(double Compliance);
	double GetCompliance() const;

	void SetElasticity(double Elasticity);
	double GetElasticity() const;

	void SetSpookDamping(double SpookDamping);
	double GetSpookDamping() const;

	void SetForceRange(FAGX_RealInterval ForceRange);
	FAGX_RealInterval GetForceRange() const;

	double GetForce() const;

protected:
	FConstraintControllerBarrier() = default;
	FConstraintControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

private:
	FConstraintControllerBarrier(const FConstraintControllerBarrier&) = delete;
	void operator=(const FConstraintControllerBarrier&) = delete;

protected:
	std::unique_ptr<FConstraintControllerRef> NativeRef;
};

class AGXUNREALBARRIER_API FElectricMotorControllerBarrier : public FConstraintControllerBarrier
{
public:
	FElectricMotorControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetVoltage(double Voltage);
	double GetVoltage() const;

	void SetArmatureResistance(double ArmatureResistance);
	double GetArmatureResistance() const;

	void SetTorqueConstant(double TorqueConstant);
	double GetTorqueConstant() const;
};

class AGXUNREALBARRIER_API FFrictionControllerBarrier : public FConstraintControllerBarrier
{
public:
	FFrictionControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetFrictionCoefficient(double FrictionCoefficient);
	double GetFrictionCoefficient() const;

	void SetEnableNonLinearDirectSolveUpdate(bool bEnable);
	bool GetEnableNonLinearDirectSolveUpdate() const;
};

class AGXUNREALBARRIER_API FLockControllerBarrier : public FConstraintControllerBarrier
{
public:
	FLockControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetPositionTranslational(double Position);
	double GetPositionTranslational() const;

	void SetPositionRotational(double Position);
	double GetPositionRotational() const;
};

class AGXUNREALBARRIER_API FRangeControllerBarrier : public FConstraintControllerBarrier
{
public:
	FRangeControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetRangeTranslational(FAGX_RealInterval Range);
	FAGX_RealInterval GetRangeTranslational() const;

	void SetRangeRotational(FAGX_RealInterval Range);
	FAGX_RealInterval GetRangeRotational() const;
};

class AGXUNREALBARRIER_API FScrewControllerBarrier : public FConstraintControllerBarrier
{
public:
	FScrewControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetLead(double Lead);
	double GetLead() const;
};

class AGXUNREALBARRIER_API FTargetSpeedControllerBarrier : public FConstraintControllerBarrier
{
public:
	FTargetSpeedControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetSpeedTranslational(double Speed);
	double GetSpeedTranslational() const;

	void SetSpeedRotational(double Speed);
	double GetSpeedRotational() const;

	void SetLockedAtZeroSpeed(bool LockedAtZeroSpeed);
	bool GetLockedAtZeroSpeed() const;
};

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier
{
public: // Special member functions.
	/// Create a Barrier with a Ref but without a Native.
	FTwistRangeControllerBarrier();
	/// Create a Barrier with the same Native as Other, but a separate Ref.
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);
	/// Create a Barrier that takes over ownership of the given Ref.
	FTwistRangeControllerBarrier(std::unique_ptr<FTwistRangeControllerRef> Native);
	/**
	 * Destructor must be virtual since we (will) inherit and must be non-inline because the
	 * Ref type definition isn't known here so the std::unique_ptr destructor cannot be
	 * instantiated.
	 */
	virtual ~FTwistRangeControllerBarrier();

	/**
	 * Make this Barrier point to the same Native object as Other. The Ref object will not be
	 * shared, only the AGX Dynamics object.
	 *
	 * If this was the last reference to the current Native object then it will be deleted.
	 */
	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

public: // AGX Dynamics accessors.
	void SetEnabled(bool bInEnabled);
	bool GetEnabled() const;

	void SetRange(FDoubleInterval InRange);
	FDoubleInterval GetRange() const;

public: // Native management.
	bool HasNative() const;
	FTwistRangeControllerRef* GetNative();
	const FTwistRangeControllerRef* GetNative() const;

protected:
	std::unique_ptr<FTwistRangeControllerRef> NativeRef;
};
