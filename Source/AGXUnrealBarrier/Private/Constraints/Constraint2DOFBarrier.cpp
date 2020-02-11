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

namespace
{
	template <typename FBarrier>
	agx::BasicControllerConstraint* GetNativeController(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		/// \todo Better error message here.
		UE_LOG(LogAGX, Error, TEXT("Hit base implementation of GetNativeController. This is a bug."));
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
	agx::BasicControllerConstraint* GetNativeController<FScrewControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getScrew1D();
	}

	template <>
	agx::BasicControllerConstraint* GetNativeController<FTargetSpeedControllerBarrier>(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		return Constraint->getMotor1D(Dof);
	}

	template <typename FBarrier>
	TUniquePtr<FBarrier> CreateControllerBarrier(
		const FConstraint2DOFBarrier& Barrier, EAGX_Constraint2DOFFreeDOF Dof)
	{
		agx::Constraint2DOF* Native =
			dynamic_cast<agx::Constraint2DOF*>(Barrier.GetNative()->Native.get());

		return TUniquePtr<FBarrier>(new FBarrier(std::make_unique<FConstraintControllerRef>(
			GetNativeController<FBarrier>(Native, Convert(Dof)))));
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
