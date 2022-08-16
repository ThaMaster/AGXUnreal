// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_WireMergeSplitThresholdsBase.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireMergeSplitThresholdsInstance.generated.h"

class UAGX_WireMergeSplitThresholdsAsset;

UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_WireMergeSplitThresholdsInstance
	: public UAGX_WireMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_WireMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

	void CreateNative(UWorld* PlayingWorld);
	bool HasNative();
	FWireMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_WireMergeSplitThresholdsInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_WireMergeSplitThresholdsAsset& Source);

private:
	void CopyProperties(UAGX_WireMergeSplitThresholdsAsset& Source);
	void UpdateNativeProperties();

	TUniquePtr<FWireMergeSplitThresholdsBarrier> NativeBarrier;
};