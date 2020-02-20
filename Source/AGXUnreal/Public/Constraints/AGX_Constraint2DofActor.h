#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Constraint2DofActor.generated.h"

class UAGX_Constraint2DofComponent;

UCLASS(ClassGroup = "AGX", Abstract, NotBlueprintable)
class AGXUNREAL_API AAGX_Constraint2DofActor : public AAGX_ConstraintActor
{
	GENERATED_BODY()

public:
	AAGX_Constraint2DofActor();
	virtual ~AAGX_Constraint2DofActor();

	UAGX_Constraint2DofComponent* Get2DofComponent();

protected:
	AAGX_Constraint2DofActor(UAGX_Constraint2DofComponent* InConstraintComponent);
};
