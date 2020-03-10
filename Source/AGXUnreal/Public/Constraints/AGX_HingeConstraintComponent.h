#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_HingeConstraintComponent.generated.h"

/**
 * Locks all degrees of freedom except for rotation around the Z-axis.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_HingeConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	UAGX_HingeConstraintComponent();
	virtual ~UAGX_HingeConstraintComponent() override;

protected:
	virtual void AllocateNative() override;
};
