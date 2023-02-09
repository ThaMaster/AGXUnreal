// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BallConstraintComponent.generated.h"

class FBallJointBarrier;

/**
 * Locks all translational degrees of freedom, but rotation is free.
 */
UCLASS(ClassGroup = "AGX_Constraint", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BallConstraintComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FBallJointBarrier;

public:
	UAGX_BallConstraintComponent();
	virtual ~UAGX_BallConstraintComponent() override;

	FBallJointBarrier* GetNativeBallJoint();
	const FBallJointBarrier* GetNativeBallJoint() const;

protected:
	virtual void CreateNativeImpl() override;
};
