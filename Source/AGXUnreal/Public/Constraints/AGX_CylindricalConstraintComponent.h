#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint2DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CylindricalConstraintComponent.generated.h"

/**
 * Locks all degrees of freedom except for translation and rotation along/around the Z-axis.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CylindricalConstraintComponent : public UAGX_Constraint2DofComponent
{
	GENERATED_BODY()

public:
	UAGX_CylindricalConstraintComponent();
	virtual ~UAGX_CylindricalConstraintComponent() override;

protected:
	virtual void AllocateNative() override;
};
