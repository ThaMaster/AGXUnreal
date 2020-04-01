#include "Constraints/Constraint2DOFBarrier.h"

// AGXUnreal includes.
#include "AGXRefs.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

FConstraint2DOFBarrier::FConstraint2DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint2DOFBarrier::FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint2DOFBarrier::~FConstraint2DOFBarrier()
{
}

// A collection of template specializations to fetch the AGX Dynamics controller constraint
// from a AGX Dynamics Constraint2DOF. Each template specialization knows if getMotor1D, getRange1D,
// or something else should be called for the particular Barrier type that the template is
// specialized for.
namespace
{
	agx::Constraint2DOF* Get2DOF(std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	}

	agx::Constraint2DOF* Get2DOF(const std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	}

	// Base implementation for the controller constraints for which there are two per constraint.
	// Should never get here.
	/// \todo Can we enforce this at compile time?
	template <typename FBarrier>
	agx::BasicControllerConstraint* GetNativeController(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Failed to get secondary constraint for 2-DOF constraint '%s'."),
			*Convert(Constraint->getName()));
		return nullptr;
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FElectricMotorControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		check(Constraint->getElectricMotorController(Dof) != nullptr);
		return Constraint->getElectricMotorController(Dof);
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FFrictionControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getFrictionController(Dof);
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FLockControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getLock1D(Dof);
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FRangeControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getRange1D(Dof);
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FTargetSpeedControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getMotor1D(Dof);
	}

	// Base implementation for the controller constraints for which there are one per constraint.
	// Should never get here.
	/// \todo Can we enforce this at compile time?
	template <typename FBarrier>
	agx::BasicControllerConstraint* GetNativeController(agx::Constraint2DOF* Constraint)
	{
		/// \todo Better error message here.
		UE_LOG(
			LogAGX, Error, TEXT("Failed to get secondary constraint for 2-DOF constraint '%s'."));
		return nullptr;
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FScrewControllerBarrier>(
		agx::Constraint2DOF* Constraint)
	{
		return Constraint->getScrew1D();
	}

	// Dispatch function that extracts the AGX Dynamics Constraint2DOF and then uses the template
	// overload set above to get the proper constraint controller from it.
	// Call this for all constraint controller types for which Constraint2DOF has two controllers.
	// FBarrier should be one of the constraint controller barrier types, e.g.,
	// FRangeControllerBarrier.
	template <typename FBarrier>
	TUniquePtr<FBarrier> CreateControllerBarrier(
		const FConstraint2DOFBarrier& Barrier, EAGX_Constraint2DOFFreeDOF Dof)
	{
		agx::Constraint2DOF* Native =
			dynamic_cast<agx::Constraint2DOF*>(Barrier.GetNative()->Native.get());

		return TUniquePtr<FBarrier>(new FBarrier(std::make_unique<FConstraintControllerRef>(
			GetNativeController<FBarrier>(Native, Convert(Dof)))));
	}

	// Dispatch function that extracts the AGX Dynamics Constraint2DOF and then uses the template
	// overload set above to get the proper constraint controller from it.
	// Call this for all constraint controller types for which Constraint2DOF has only one
	// controller.
	// FBarrier should be one of the constraint controller barrier types, e.g.,
	// FScrewControllerBarrier.
	template <typename FBarrier>
	TUniquePtr<FBarrier> CreateControllerBarrier(const FConstraint2DOFBarrier& Barrier)
	{
		check(Barrier.HasNative());
		agx::Constraint2DOF* Native =
			dynamic_cast<agx::Constraint2DOF*>(Barrier.GetNative()->Native.get());
		check(Native != nullptr);

		return TUniquePtr<FBarrier>(new FBarrier(
			std::make_unique<FConstraintControllerRef>(GetNativeController<FBarrier>(Native))));
	}
}

TUniquePtr<FElectricMotorControllerBarrier> FConstraint2DOFBarrier::GetElectricMotorController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FElectricMotorControllerBarrier>(*this, Dof);
}

TUniquePtr<FFrictionControllerBarrier> FConstraint2DOFBarrier::GetFrictionController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FFrictionControllerBarrier>(*this, Dof);
}

TUniquePtr<FLockControllerBarrier> FConstraint2DOFBarrier::GetLockController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FLockControllerBarrier>(*this, Dof);
}

TUniquePtr<FRangeControllerBarrier> FConstraint2DOFBarrier::GetRangeController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FRangeControllerBarrier>(*this, Dof);
}

TUniquePtr<FTargetSpeedControllerBarrier> FConstraint2DOFBarrier::GetTargetSpeedController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(*this, Dof);
}

TUniquePtr<FScrewControllerBarrier> FConstraint2DOFBarrier::GetScrewController()
{
	return CreateControllerBarrier<FScrewControllerBarrier>(*this);
}

TUniquePtr<const FElectricMotorControllerBarrier>
FConstraint2DOFBarrier::GetElectricMotorController(EAGX_Constraint2DOFFreeDOF Dof) const
{
	// return CreateControllerBarrier<const FElectricMotorControllerBarrier>(*this, Dof);
	agx::ElectricMotorController* Controller =
		Get2DOF(NativeRef)->getElectricMotorController(Convert(Dof));
	return TUniquePtr<const FElectricMotorControllerBarrier>(new FElectricMotorControllerBarrier(
		std::make_unique<FConstraintControllerRef>(Controller)));
}

namespace
{
	agx::FrictionController* GetOrCreateFrictionController(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		agx::FrictionController* Friction = Constraint->getFrictionController(Dof);
		if (Friction == nullptr)
		{
			agx::AttachmentPair* Attachments = Constraint->getAttachmentPair();
			agx::Angle* SepAngle = Attachments->getAngle(0);
			agx::Angle* RotAngle = Attachments->getAngle(1);
			agx::ConstraintAngleBasedData SepData(Attachments, SepAngle);
			agx::ConstraintAngleBasedData RotData(Attachments, RotAngle);
			agx::FrictionController* SepFriction = new agx::FrictionController(SepData);
			agx::FrictionController* RotFriction = new agx::FrictionController(RotData);
			Constraint->addSecondaryConstraint("FT", SepFriction);
			Constraint->addSecondaryConstraint("FR", RotFriction);
			Friction = Dof == agx::Constraint2DOF::FIRST ? SepFriction : RotFriction;
			check(Friction == Constraint->getFrictionController(Dof));
		}
		return Friction;
	}
}

TUniquePtr<const FFrictionControllerBarrier> FConstraint2DOFBarrier::GetFrictionController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	check(HasNative());
	agx::FrictionController* Controller =
		GetOrCreateFrictionController(Get2DOF(NativeRef), Convert(Dof));
	return TUniquePtr<const FFrictionControllerBarrier>(
		new FFrictionControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));

	return CreateControllerBarrier<const FFrictionControllerBarrier>(*this, Dof);
}

TUniquePtr<const FLockControllerBarrier> FConstraint2DOFBarrier::GetLockController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	// return CreateControllerBarrier<const FLockControllerBarrier>(*this, Dof);
	agx::LockController* Controller = Get2DOF(NativeRef)->getLock1D(Convert(Dof));
	return TUniquePtr<const FLockControllerBarrier>(
		new FLockControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));
}

TUniquePtr<const FRangeControllerBarrier> FConstraint2DOFBarrier::GetRangeController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	// return CreateControllerBarrier<const FRangeControllerBarrier>(*this, Dof);
	agx::RangeController* Controller = Get2DOF(NativeRef)->getRange1D(Convert(Dof));
	return TUniquePtr<const FRangeControllerBarrier>(
		new FRangeControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));
}

TUniquePtr<const FTargetSpeedControllerBarrier> FConstraint2DOFBarrier::GetTargetSpeedController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	// return CreateControllerBarrier<const FTargetSpeedControllerBarrier>(*this, Dof);
	agx::TargetSpeedController* Controller = Get2DOF(NativeRef)->getMotor1D(Convert(Dof));
	return TUniquePtr<const FTargetSpeedControllerBarrier>(
		new FTargetSpeedControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));
}

TUniquePtr<const FScrewControllerBarrier> FConstraint2DOFBarrier::GetScrewController() const
{
	// return CreateControllerBarrier<const FScrewControllerBarrier>(*this);
	agx::ScrewController* Controller = Get2DOF(NativeRef)->getScrew1D();
	return TUniquePtr<const FScrewControllerBarrier>(
		new FScrewControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));
}
