#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BallConstraintComponent.generated.h"

/**
 * Locks all translational degrees of freedom, but rotation is free.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BallConstraintComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	UAGX_BallConstraintComponent();
	virtual ~UAGX_BallConstraintComponent() override;

protected:
	virtual void CreateNativeImpl() override;
};
