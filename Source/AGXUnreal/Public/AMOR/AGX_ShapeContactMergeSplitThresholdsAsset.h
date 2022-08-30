// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholdsAsset.generated.h"

class UAGX_ShapeContactMergeSplitThresholdsInstance;


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholdsAsset
	: public UAGX_ShapeContactMergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	virtual UAGX_ShapeContactMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld) override;

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
	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholdsInstance> Instance;
};