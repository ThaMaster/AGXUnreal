#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceConstraintComponent.generated.h"

class FDistanceJointBarrier;

/**
 * Locks the initial relative distance between the bodies.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DistanceConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FDistanceJointBarrier;

public:
	UAGX_DistanceConstraintComponent();
	virtual ~UAGX_DistanceConstraintComponent();

	FDistanceJointBarrier* GetNativeDistance();
	const FDistanceJointBarrier* GetNativeDistance() const;

protected:
	virtual void AllocateNative() override;
};
