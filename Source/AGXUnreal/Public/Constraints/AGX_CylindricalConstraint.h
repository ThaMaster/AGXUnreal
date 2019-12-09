// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint2DOF.h"
#include "AGX_CylindricalConstraint.generated.h"

/**
 * Locks all degrees of freedom except for translation and rotation along/around the Z-axis.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_CylindricalConstraint : public AAGX_Constraint2DOF
{
	GENERATED_BODY()

public:
	AAGX_CylindricalConstraint();
	virtual ~AAGX_CylindricalConstraint();

protected:
	virtual void CreateNativeImpl() override;
};
