#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Constraint2DofComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_Constraint2DofComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	/** Electric motor controller for first secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintElectricMotorController ElectricMotorController1;

	/** Electric motor controller for second secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintElectricMotorController ElectricMotorController2;

	/** Friction controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintFrictionController FrictionController1;

	/** Friction controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintFrictionController FrictionController2;

	/** Lock controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintLockController LockController1;

	/** Lock controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintLockController LockController2;

	/** Range controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController1;

	/** Range controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController2;

	/** Target speed controller for first secondary constraint (on one of the 2 free DOFs, usually).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintTargetSpeedController TargetSpeedController1;

	/** Target speed controller for second secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintTargetSpeedController TargetSpeedController2;

	/** Screw controller that puts a relationship between the two free DOFs. */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintScrewController ScrewController;

	UAGX_Constraint2DofComponent();

	UAGX_Constraint2DofComponent(
		const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational,
		bool bIsSecondaryConstraint2Rotational);

	virtual ~UAGX_Constraint2DofComponent();

	virtual void UpdateNativeProperties() override;

private:
	class FConstraint2DOFBarrier* GetNativeBarrierCasted() const;
};
