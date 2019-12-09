// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint1DOF.h"
#include "AGX_DistanceConstraint.generated.h"

/**
 * Locks the initial relative distance between the bodies.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_DistanceConstraint : public AAGX_Constraint1DOF
{
	GENERATED_BODY()

public:
	AAGX_DistanceConstraint();
	virtual ~AAGX_DistanceConstraint();

protected:
	virtual void CreateNativeImpl() override;
};
