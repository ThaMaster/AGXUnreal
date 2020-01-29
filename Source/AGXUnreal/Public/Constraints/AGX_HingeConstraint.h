// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint1DOF.h"
#include "AGX_HingeConstraint.generated.h"

/**
 * Locks all degrees of freedom except for rotation around the Z-axis.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_HingeConstraint : public AAGX_Constraint1DOF
{
	GENERATED_BODY()

public:
	AAGX_HingeConstraint();
	virtual ~AAGX_HingeConstraint();

protected:
	virtual void CreateNativeImpl() override;
};
