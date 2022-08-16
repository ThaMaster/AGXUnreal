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
		UWorld* PlayingWorld) override;

	void CreateNative(UWorld* PlayingWorld);
	bool HasNative();
	FConstraintMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_ConstraintMergeSplitThresholdsInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholdsAsset& Source);

private:
	void CopyProperties(UAGX_ConstraintMergeSplitThresholdsAsset& Source);
	void UpdateNativeProperties();

	TUniquePtr<FConstraintMergeSplitThresholdsBarrier> NativeBarrier;
};