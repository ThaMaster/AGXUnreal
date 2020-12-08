#include "Constraints/Constraint1DOFBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

// Standard library includes.
#include <memory>

FConstraint1DOFBarrier::FConstraint1DOFBarrier()
{
}

FConstraint1DOFBarrier::FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint1DOFBarrier::~FConstraint1DOFBarrier()
{
}

namespace
{
	agx::Constraint1DOF* Get1DOF(std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	}

	agx::Constraint1DOF* Get1DOF(const std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	}

	template <typename Barrier>
	TUniquePtr<Barrier> CreateControllerBarrier(agx::BasicControllerConstraint* Controller)
	{
		return TUniquePtr<Barrier>(
			new Barrier(std::make_unique<FConstraintControllerRef>(Controller)));
	}
}

float FConstraint1DOFBarrier::GetAngle() const
{
	/// @TODO Convert from AGX Dynamics units to Unreal Engine units. Difficult because we could
	/// be a constraint with either a free rotational degree of freedom or a free translational
	/// degree of freedom.
	check(HasNative());

	const agx::Constraint1DOF* Constraint = Get1DOF(NativeRef);
	const agx::Real NativeAngle = Constraint->getAngle();
	const agx::Motor1D* Motor = Constraint->getMotor1D();
	if (Motor == nullptr)
	{
		return NativeAngle;
	}
	const agx::Angle* AngleType = Motor->getData().getAngle();
	if (AngleType == nullptr)
	{
		return NativeAngle;
	}
	const agx::Angle::Type DofType = AngleType->getType();
	switch (DofType)
	{
		case agx::Angle::ROTATIONAL:
			return ConvertAngle(NativeAngle);
		case agx::Angle::TRANSLATIONAL:
			ConvertDistance(NativeAngle);
		default:
			return NativeAngle;
	}
}

TUniquePtr<FElectricMotorControllerBarrier> FConstraint1DOFBarrier::GetElectricMotorController()
{
	check(HasNative());
	return CreateControllerBarrier<FElectricMotorControllerBarrier>(
		Get1DOF(NativeRef)->getElectricMotorController());
}

TUniquePtr<FFrictionControllerBarrier> FConstraint1DOFBarrier::GetFrictionController()
{
	check(HasNative());
	return CreateControllerBarrier<FFrictionControllerBarrier>(
		Get1DOF(NativeRef)->getFrictionController());
}

TUniquePtr<FLockControllerBarrier> FConstraint1DOFBarrier::GetLockController()
{
	check(HasNative());
	return CreateControllerBarrier<FLockControllerBarrier>(Get1DOF(NativeRef)->getLock1D());
}

TUniquePtr<FRangeControllerBarrier> FConstraint1DOFBarrier::GetRangeController()
{
	check(HasNative());
	return CreateControllerBarrier<FRangeControllerBarrier>(Get1DOF(NativeRef)->getRange1D());
}

TUniquePtr<FTargetSpeedControllerBarrier> FConstraint1DOFBarrier::GetTargetSpeedController()
{
	check(HasNative());
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(Get1DOF(NativeRef)->getMotor1D());
}

TUniquePtr<const FElectricMotorControllerBarrier>
FConstraint1DOFBarrier::GetElectricMotorController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FElectricMotorControllerBarrier>(
		Get1DOF(NativeRef)->getElectricMotorController());
}

namespace
{
	agx::FrictionController* GetOrCreateFrictionController(agx::Constraint1DOF* Constraint)
	{
		agx::FrictionController* Friction = Constraint->getFrictionController();
		if (Friction == nullptr)
		{
			agx::AttachmentPair* Attachments = Constraint->getAttachmentPair();
			agx::Angle* Angle = Attachments->getAngle(0);
			agx::ConstraintAngleBasedData AngleData(Attachments, Angle);
			Friction = new agx::FrictionController(AngleData);
			Constraint->addSecondaryConstraint("FT", Friction);
			check(Friction == Constraint->getFrictionController());
		}
		return Friction;
	}
}

TUniquePtr<const FFrictionControllerBarrier> FConstraint1DOFBarrier::GetFrictionController() const
{
	check(HasNative());
	agx::FrictionController* Friction = GetOrCreateFrictionController(Get1DOF(NativeRef));
	return CreateControllerBarrier<const FFrictionControllerBarrier>(Friction);
}

TUniquePtr<const FLockControllerBarrier> FConstraint1DOFBarrier::GetLockController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FLockControllerBarrier>(Get1DOF(NativeRef)->getLock1D());
}

TUniquePtr<const FRangeControllerBarrier> FConstraint1DOFBarrier::GetRangeController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FRangeControllerBarrier>(Get1DOF(NativeRef)->getRange1D());
}

TUniquePtr<const FTargetSpeedControllerBarrier> FConstraint1DOFBarrier::GetTargetSpeedController()
	const
{
	check(HasNative());
	return CreateControllerBarrier<const FTargetSpeedControllerBarrier>(
		Get1DOF(NativeRef)->getMotor1D());
}
