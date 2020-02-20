#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Constraint1DofComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_Constraint1DofComponent : public UAGX_ConstraintComponent
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

	UAGX_Constraint1DofComponent();

	UAGX_Constraint1DofComponent(
		const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
		bool bIsLockControllerEditable = true);

	virtual ~UAGX_Constraint1DofComponent();

	virtual void UpdateNativeProperties() override;

protected:
	virtual void CreateNativeImpl() override final;
	virtual void AllocateNative() PURE_VIRTUAL(UAGX_Constraint1DofComponent::AllocateNative, );

private:
	class FConstraint1DOFBarrier* GetNativeBarrierCasted() const;

	UPROPERTY(Transient, Category = "AGX Secondary Constraint", VisibleDefaultsOnly)
	bool bIsLockControllerEditable;
};
