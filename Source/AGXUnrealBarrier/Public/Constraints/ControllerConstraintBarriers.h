// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/ElementaryConstraintBarrier.h"

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

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier : public FElementaryConstraintBarrier
{
public:
	FTwistRangeControllerBarrier();
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);
	FTwistRangeControllerBarrier(std::unique_ptr<FTwistRangeControllerRef> Native);
	virtual ~FTwistRangeControllerBarrier();

	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

	void SetRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetRange() const;

	bool HasNative() const;
	FTwistRangeControllerRef* GetNative();
	const FTwistRangeControllerRef* GetNative() const;

protected:
	/**
	 * Type-specific native handle. Always points to the same AGX Dynamics object as the handle in
	 * the base class. The unique_ptr may never be nullptr.
	 */
	std::unique_ptr<FTwistRangeControllerRef> NativeRef;
};
