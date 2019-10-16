// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint.h"
#include "AGX_BallConstraint.generated.h"


/**
 * A constraint that locks the three tranlsations degrees of freedom between two Rigid Bodies.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_BallConstraint : public AAGX_Constraint
{
	GENERATED_BODY()

public:

	AAGX_BallConstraint();
	virtual ~AAGX_BallConstraint();

protected:

	virtual void CreateNativeImpl() override;
};
