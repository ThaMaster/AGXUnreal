#pragma once

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

struct FConstraintControllerRef;

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

	void SetForceRange(FFloatInterval ForceRange);
	FFloatInterval GetForceRange() const;

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

	void SetRangeTranslational(FFloatInterval Range);
	FFloatInterval GetRangeTranslational() const;

	void SetRangeRotational(FFloatInterval Range);
	FFloatInterval GetRangeRotational() const;
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
