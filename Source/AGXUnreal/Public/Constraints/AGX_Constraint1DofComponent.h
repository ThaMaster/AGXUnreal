#pragma once

// AGX Dynamics for Unreal includes.
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintElectricMotorController ElectricMotorController;

	/** Friction controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintFrictionController FrictionController;

	/** Lock controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint",
		Meta = (EditCondition = "bIsLockControllerEditable"))
	FAGX_ConstraintLockController LockController;

	/** Range controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintRangeController RangeController;

	/** Target speed controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintTargetSpeedController TargetSpeedController;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Constraint")
	float GetAngle() const;

	UAGX_Constraint1DofComponent();

	UAGX_Constraint1DofComponent(
		const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
		bool bIsLockControllerEditable = true);

	virtual ~UAGX_Constraint1DofComponent() override;

	virtual void UpdateNativeProperties() override;

	// ~Begin IAGX_NativeOwner interface.
	virtual void AssignNative(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	// ~ Begin UObject interface.
	virtual void PostLoad() override;
	// ~ End UObject interface.
protected:
	/**
	 * Call AllocateNative and then bind the constraint controllers to their native representations
	 * within the allocated native constraint.
	 */
	virtual void CreateNativeImpl() override final;

	/**
	 * Allocate the native constraint, of the appropriate type for the current subclass of
	 * UAGX_Constraint1DofComponent, and assign the new typed ConstraintBarrier to the inherited
	 * NativeBarrier member variable.
	 */
	virtual void AllocateNative() PURE_VIRTUAL(UAGX_Constraint1DofComponent::AllocateNative, );

private:
	// Some constraints, such as Distance, cannot use the lock constraint because it is reserved for
	// internal use within the native constraint.
	UPROPERTY(Transient, Category = "AGX Secondary Constraint", VisibleDefaultsOnly)
	bool bIsLockControllerEditable;
};
