#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Enine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceConstraintComponent.generated.h"

/**
 * Locks the initial relative distance between the bodies.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DistanceConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	UAGX_DistanceConstraintComponent();
	virtual ~UAGX_DistanceConstraintComponent();

protected:
	virtual void AllocateNative() override;
};
