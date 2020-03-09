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
		agx::Constraint2DOF* Native =
			dynamic_cast<agx::Constraint2DOF*>(Barrier.GetNative()->Native.get());

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
	return CreateControllerBarrier<const FElectricMotorControllerBarrier>(*this, Dof);
}

TUniquePtr<const FFrictionControllerBarrier> FConstraint2DOFBarrier::GetFrictionController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FFrictionControllerBarrier>(*this, Dof);
}

TUniquePtr<const FLockControllerBarrier> FConstraint2DOFBarrier::GetLockController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FLockControllerBarrier>(*this, Dof);
}

TUniquePtr<const FRangeControllerBarrier> FConstraint2DOFBarrier::GetRangeController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FRangeControllerBarrier>(*this, Dof);
}

TUniquePtr<const FTargetSpeedControllerBarrier> FConstraint2DOFBarrier::GetTargetSpeedController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FTargetSpeedControllerBarrier>(*this, Dof);
}

TUniquePtr<const FScrewControllerBarrier> FConstraint2DOFBarrier::GetScrewController() const
{
	return CreateControllerBarrier<const FScrewControllerBarrier>(*this);
}
