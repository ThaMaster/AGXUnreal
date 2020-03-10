#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceConstraintActor.generated.h"

UCLASS(ClassGroup = "AGX", Blueprintable)
class AGXUNREAL_API AAGX_DistanceConstraintActor : public AAGX_Constraint1DofActor
{
	GENERATED_BODY()

public:
	AAGX_DistanceConstraintActor();
	virtual ~AAGX_DistanceConstraintActor() override;
};
