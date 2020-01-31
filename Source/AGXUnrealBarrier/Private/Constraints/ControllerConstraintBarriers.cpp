#include "Constraints/ControllerConstraintBarriers.h"

// Unreal Engine includes.
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

FConstraintControllerBarrier::FConstraintControllerBarrier(FConstraintControllerBarrier&& Other) noexcept
	: NativeRef(std::move(Other.NativeRef))
{
	check(NativeRef.get());
}

FConstraintControllerBarrier::FConstraintControllerBarrier(
	std::unique_ptr<FConstraintControllerRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef.get());
}

FConstraintControllerBarrier::~FConstraintControllerBarrier()
{
}

FConstraintControllerBarrier& FConstraintControllerBarrier::operator=(
	FConstraintControllerBarrier&& Other) noexcept
{
	NativeRef = std::move(Other.NativeRef);
	check(NativeRef.get());
	return *this;
}

bool FConstraintControllerBarrier::HasNative() const {
	return NativeRef->Native != nullptr;
}

FConstraintControllerRef* FConstraintControllerBarrier::GetNative()
{
	return NativeRef.get();
}

const FConstraintControllerRef* FConstraintControllerBarrier::GetNative() const
{
	return NativeRef.get();
}

void FConstraintControllerBarrier::SetCompliance(float Compliance)
{
	check(HasNative());
	const float ComplianceAGX = Convert(Compliance);
	NativeRef->Native->setCompliance(ComplianceAGX);
}

float FConstraintControllerBarrier::GetCompliance() const
{
	check(HasNative());
	const float ComplianceAGX = NativeRef->Native->getCompliance();
	return Convert(ComplianceAGX);
}

void FConstraintControllerBarrier::SetDamping(float Damping)
{
	check(HasNative());
	const float DampingAGX = Convert(Damping);
	NativeRef->Native->setDamping(DampingAGX);
}

float FConstraintControllerBarrier::GetDamping() const
{
	check(HasNative());
	const float DampingAGX = NativeRef->Native->getDamping();
	return Convert(DampingAGX);
}

void FConstraintControllerBarrier::SetForceRange(FFloatInterval ForceRange)
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = Convert(ForceRange);
	NativeRef->Native->setForceRange(ForceRangeAGX);
}

FFloatInterval FConstraintControllerBarrier::GetForceRange() const
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = NativeRef->Native->getForceRange();
	return Convert(ForceRangeAGX);
}




FElectricMotorControllerBarrier

#if 0
void FElectricMotorControllerBarrier::ToNative(agx::ElectricMotorController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setVoltage(Voltage);
	Native->setArmatureResistance(ArmatureResistance);
	Native->setTorqueConstant(TorqueConstant);
}

void FFrictionControllerBarrier::ToNative(agx::FrictionController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setFrictionCoefficient(FrictionCoefficient);
	Native->setEnableNonLinearDirectSolveUpdate(bEnableNonLinearDirectSolveUpdate);
}

void FLockControllerBarrier::ToNative(agx::LockController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setPosition(bRotational ? Position : ConvertDistanceToAgx(Position));
}

void FRangeControllerBarrier::ToNative(agx::RangeController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setRange(agx::RangeReal(
		bRotational ? RangeMin : ConvertDistanceToAgx(RangeMin),
		bRotational ? RangeMax : ConvertDistanceToAgx(RangeMax)));
}

void FScrewControllerBarrier::ToNative(agx::ScrewController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setLead(ConvertDistanceToAgx(Lead));
}

void FTargetSpeedControllerBarrier::ToNative(agx::TargetSpeedController* Native) const
{
	// Common controller variables.
	Native->setEnable(bEnable);
	Native->setElasticity(Elasticity);
	Native->setDamping(Damping);
	Native->setForceRange(agx::RangeReal(ForceRangeMin, ForceRangeMax));

	// Special controller variables.
	Native->setSpeed(bRotational ? Speed : ConvertDistanceToAgx(Speed));
	Native->setLockedAtZeroSpeed(bLockedAtZeroSpeed);
}
#endif
