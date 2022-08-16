// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsBase.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholdsInstance.generated.h"

class UAGX_ShapeContactMergeSplitThresholdsAsset;

UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholdsInstance
	: public UAGX_ShapeContactMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_ShapeContactMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

	void CreateNative(UWorld* PlayingWorld);
	bool HasNative();
	FShapeContactMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_ShapeContactMergeSplitThresholdsInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholdsAsset& Source);

private:
	void CopyProperties(UAGX_ShapeContactMergeSplitThresholdsAsset& Source);
	void UpdateNativeProperties();

	TUniquePtr<FShapeContactMergeSplitThresholdsBarrier> NativeBarrier;
};