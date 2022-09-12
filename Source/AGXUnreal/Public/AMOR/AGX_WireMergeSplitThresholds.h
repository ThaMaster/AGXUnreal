// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireMergeSplitThresholds.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_WireMergeSplitThresholds : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * When external forces are acting on a partially merged wire, the force will propagate and
	 * split several nodes at once. This threshold controls the amount of force (of the total
	 * external force) that is used to split each node.
	 * If this value is high (> 1), the force
	 * will not propagate 'too' long, keeping the wire merged.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Merge Split Thresholds")
	FAGX_Real ForcePropagationDecayScale {1.0};

	UFUNCTION(BlueprintCallable, Category = "Wire Merge Split Thresholds")
	void SetForcePropagationDecayScale_BP(float InForcePropagationDecayScale);
	virtual void SetForcePropagationDecayScale(FAGX_Real InForcePropagationDecayScale);

	UFUNCTION(BlueprintCallable, Category = "Wire Merge Split Thresholds")
	float GetForcePropagationDecayScale_BP() const;
	virtual FAGX_Real GetForcePropagationDecayScale() const;

	/**
	 * When a node is merged the tension is stored and monitored to perform split. This threshold
	 * scales the merge tension making split more likely when < 1 and less likely > 1. When this
	 * scale is 0 the only force keeping the node merged is from the contact - which in most cases
	 * isn't enough.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Merge Split Thresholds")
	FAGX_Real MergeTensionScale {1.0};

	UFUNCTION(BlueprintCallable, Category = "Wire Merge Split Thresholds")
	void SetMergeTensionScale_BP(float InMergeTensionScale);
	virtual void SetMergeTensionScale(FAGX_Real InMergeTensionScale);

	UFUNCTION(BlueprintCallable, Category = "Wire Merge Split Thresholds")
	float GetMergeTensionScale_BP() const;
	virtual FAGX_Real GetMergeTensionScale() const;

	void CreateNative(UWorld* PlayingWorld);
	bool HasNative() const;
	FWireMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_WireMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_WireMergeSplitThresholds& Source);

	UAGX_WireMergeSplitThresholds* GetOrCreateInstance(UWorld* PlayingWorld);

	bool IsInstance() const;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CopyProperties(UAGX_WireMergeSplitThresholds& Source);
	void UpdateNativeProperties();

	TWeakObjectPtr<UAGX_WireMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_WireMergeSplitThresholds> Instance;
	TUniquePtr<FWireMergeSplitThresholdsBarrier> NativeBarrier;
};