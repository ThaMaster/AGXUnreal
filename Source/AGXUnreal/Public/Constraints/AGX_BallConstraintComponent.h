// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_TwistRangeController.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_TwistRangeController TwistRangeController;

	using FBarrierType = FBallJointBarrier;

	UAGX_BallConstraintComponent();
	virtual ~UAGX_BallConstraintComponent() override;

	virtual void UpdateNativeProperties() override;

	FBallJointBarrier* GetNativeBallJoint();
	const FBallJointBarrier* GetNativeBallJoint() const;

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& Event) override;
#endif
	//~ End UObject interface.
protected:
	virtual void CreateNativeImpl() override;
};
