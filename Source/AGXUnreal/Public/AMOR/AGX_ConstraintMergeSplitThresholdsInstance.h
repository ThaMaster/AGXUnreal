// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ConstraintMergeSplitThresholdsBase.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholdsInstance.generated.h"

class UAGX_ConstraintMergeSplitThresholdsAsset;

UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholdsInstance
	: public UAGX_ConstraintMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_ConstraintMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld, bool bIsRotational) override;

	void CreateNative(UWorld* PlayingWorld, bool bIsRotational);
	bool HasNative() const;
	FConstraintMergeSplitThresholdsBarrier* GetOrCreateNative(
		UWorld* PlayingWorld, bool bIsRotational);

	static UAGX_ConstraintMergeSplitThresholdsInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholdsAsset& Source, bool bIsRotational);

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
	void CopyProperties(UAGX_ConstraintMergeSplitThresholdsAsset& Source);
	void UpdateNativeProperties();

	TUniquePtr<FConstraintMergeSplitThresholdsBarrier> NativeBarrier;
};