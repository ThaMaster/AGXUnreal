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
	bool HasNative() const;
	FShapeContactMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_ShapeContactMergeSplitThresholdsInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholdsAsset& Source);

	virtual void SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed) override;
	virtual FAGX_Real GetMaxImpactSpeed() const override;

	virtual void SetMaxRelativeNormalSpeed(FAGX_Real InMaxRelativeNormalSpeed) override;
	virtual FAGX_Real GetMaxRelativeNormalSpeed() const override;

	virtual void SetMaxRelativeTangentSpeed(FAGX_Real InMaxRelativeTangentSpeed) override;
	virtual FAGX_Real GetMaxRelativeTangentSpeed() const override;

	virtual void SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed) override;
	virtual FAGX_Real GetMaxRollingSpeed() const override;

	virtual void SetNormalAdhesion(FAGX_Real InNormalAdhesion) override;
	virtual FAGX_Real GetNormalAdhesion() const override;

	virtual void SetTangentialAdhesion(FAGX_Real InTangentialAdhesion) override;
	virtual FAGX_Real GetTangentialAdhesion() const override;

	virtual void SetMaySplitInGravityField(bool bInMaySplitInGravityField) override;
	virtual bool GetMaySplitInGravityField() const override;

	virtual void SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact) override;
	virtual bool GetSplitOnLogicalImpact() const override;

private:
	void CopyProperties(UAGX_ShapeContactMergeSplitThresholdsAsset& Source);
	void UpdateNativeProperties();

	TUniquePtr<FShapeContactMergeSplitThresholdsBarrier> NativeBarrier;
};