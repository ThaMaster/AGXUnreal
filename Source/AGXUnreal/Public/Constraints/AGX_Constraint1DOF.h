// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint.h"
#include "AGX_Constraint1DOF.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_Constraint1DOF : public AAGX_Constraint
{
	GENERATED_BODY()

public:

	AAGX_Constraint1DOF();

	AAGX_Constraint1DOF(const TArray<EDofFlag> &LockedDofsOrdered);

	virtual ~AAGX_Constraint1DOF();
};
