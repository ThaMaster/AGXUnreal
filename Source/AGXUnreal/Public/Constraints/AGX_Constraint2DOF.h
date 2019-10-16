// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint.h"
#include "AGX_Constraint2DOF.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_Constraint2DOF : public AAGX_Constraint
{
	GENERATED_BODY()

public:

	/** Range controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController1;

	/** Range controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController2;

	AAGX_Constraint2DOF();

	AAGX_Constraint2DOF(const TArray<EDofFlag> &LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational, bool bIsSecondaryConstraint2Rotational);

	virtual ~AAGX_Constraint2DOF();

	virtual void UpdateNativeProperties() override;

private:

	class FConstraint2DOFBarrier* GetNativeBarrierCasted() const;
};
