// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ConstraintMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholdsAsset.generated.h"

class UAGX_ConstraintMergeSplitThresholdsInstance;

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholdsAsset
	: public UAGX_ConstraintMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_ConstraintMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

	virtual void SetMaxDesiredForceRangeDiff(FAGX_Real InMaxDesiredForceRangeDiff) override;
	virtual FAGX_Real GetMaxDesiredForceRangeDiff() const override;

	virtual void SetMaxDesiredLockAngleDiff(FAGX_Real InMaxDesiredLockAngleDiff) override;
	virtual FAGX_Real GetMaxDesiredLockAngleDiff() const override;

	virtual void SetMaxDesiredRangeAngleDiff(FAGX_Real InMaxDesiredRangeAngleDiff) override;
	virtual FAGX_Real GetMaxDesiredRangeAngleDiff() const override;

	virtual void SetMaxDesiredSpeedDiff(FAGX_Real InMaxDesiredSpeedDiff) override;
	virtual FAGX_Real GetMaxDesiredSpeedDiff() const override;

	virtual void SetMaxRelativeSpeed(FAGX_Real InMaxRelativeSpeed) override;
	virtual FAGX_Real GetMaxRelativeSpeed() const override;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholdsInstance> Instance;
};