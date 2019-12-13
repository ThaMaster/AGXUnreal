// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "AGX_Constraint1DOF.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_Constraint1DOF : public AAGX_Constraint
{
	GENERATED_BODY()

public:
	/** Electric motor controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraint")
	FAGX_ConstraintElectricMotorController ElectricMotorController;

	/** Friction controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraint")
	FAGX_ConstraintFrictionController FrictionController;

	/** Lock controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(
		EditAnywhere, Category = "AGX Secondary Constraint",
		Meta = (EditCondition = "bIsLockControllerEditable"))
	FAGX_ConstraintLockController LockController;

	/** Range controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraint")
	FAGX_ConstraintRangeController RangeController;

	/** Target speed controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraint")
	FAGX_ConstraintTargetSpeedController TargetSpeedController;

	AAGX_Constraint1DOF();

	AAGX_Constraint1DOF(
		const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
		bool bIsLockControllerEditable = true);

	virtual ~AAGX_Constraint1DOF();

	virtual void UpdateNativeProperties() override;

private:
	class FConstraint1DOFBarrier* GetNativeBarrierCasted() const;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool bIsLockControllerEditable;
};
