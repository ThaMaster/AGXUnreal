#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class FConstraint1DOFBarrier;
class FConstraint2DOFBarrier;
class UAGX_Constraint1DofComponent;

struct FAGX_ConstraintElectricMotorController;
struct FAGX_ConstraintFrictionController;
struct FAGX_ConstraintLockController;
struct FAGX_ConstraintRangeController;
struct FAGX_ConstraintTargetSpeedController;

class AGXUNREAL_API FAGX_ConstraintUtilities
{
public:
	/**
	 * Copy constraint controller properties, such as enabled, compliance, force range, voltage,
	 * from the AGX Dynamics constraint to the AGXUnreal constraint.
	 * @param Component The AGXUnreal constraint to copy properties to.
	 * @param Barrier The AGX Dynamics constraint to copy properties from.
	 */
	static void StoreControllers(
		UAGX_Constraint1DofComponent& Component, const FConstraint1DOFBarrier& Barrier);

	/**
	 * Copy constraint controller properties, such as enabled, compliance, force range, voltage,
	 * from the AGX Dynamics constraint to the AGXUnreal constraint.
	 * @param Component The AGXUnreal constraint to copy properties to.
	 * @param Barrier The AGX Dynamics constraint to copy properties from.
	 */
	static void StoreControllers(
		UAGX_Constraint2DofComponent& Component, const FConstraint2DOFBarrier& Barrier);

	/**
	 * Base class overload that does nothing. Only 1Dof- and 2Dof constraints have controllers so
	 * make sure one of those overloads are called when appropriate. This overload is required
	 * because the call is made from a function template that is sometimes given a non-1/2Dof
	 * constraint.
	 * @param Component
	 * @param Barrier
	 */
	static void StoreControllers(
		UAGX_ConstraintComponent& Component, const FConstraintBarrier& Barrier);

	static void StoreElectricMotorController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller);

	static void StoreElectricMotorController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreFrictionController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller);

	static void StoreFrictionController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreLockController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller);

	static void StoreLockController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreRangeController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller);

	static void StoreRangeController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreTargetSpeedController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller);

	static void StoreTargetSpeedController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreFrame(
		const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment,
		int32 BodyIndex);

	static void StoreFrames(const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component);
};
