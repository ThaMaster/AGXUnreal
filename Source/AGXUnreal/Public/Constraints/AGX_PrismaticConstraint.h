// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constraints/AGX_Constraint1DOF.h"
#include "AGX_PrismaticConstraint.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API AAGX_PrismaticConstraint : public AAGX_Constraint1DOF
{
	GENERATED_BODY()

public:

	AAGX_PrismaticConstraint();
	virtual ~AAGX_PrismaticConstraint();

protected:

	virtual void CreateNativeImpl() override;
};
