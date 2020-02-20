#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LockConstraintComponent.generated.h"

/**
 * Locks all degrees of freedom.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_LockConstraintComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	UAGX_LockConstraintComponent();
	virtual ~UAGX_LockConstraintComponent();

protected:
	virtual void CreateNativeImpl() override;
};
