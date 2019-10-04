// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint.h"
#include "AGX_LockConstraint.generated.h"


/**
 * A constraint that locks all six relative degrees of freedom between two Rigid Bodies.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_LockConstraint : public AAGX_Constraint
{
	GENERATED_BODY()

public:

	AAGX_LockConstraint();
	virtual ~AAGX_LockConstraint();

protected:

	virtual void CreateNativeImpl() override;
};
