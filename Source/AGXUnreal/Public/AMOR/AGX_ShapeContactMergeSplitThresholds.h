// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholds.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholds : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Maximum impact speed (along a contact normal) a merged object can resist
	 * without being split [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxImpactSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxImpactSpeed_BP(float InMaxImpactSpeed);

	void SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxImpactSpeed_BP() const;

	FAGX_Real GetMaxImpactSpeed() const;

	/**
	 * Maximum speed along a contact normal for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeNormalSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeNormalSpeed_BP(float InMaxRelativeNormalSpeed);

	void SetMaxRelativeNormalSpeed(FAGX_Real InMaxRelativeNormalSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRelativeNormalSpeed_BP() const;

	FAGX_Real GetMaxRelativeNormalSpeed() const;

	/**
	 * Maximum (sliding) speed along a contact tangent for a contact to be considered resting
	 * [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeTangentSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeTangentSpeed_BP(float InMaxRelativeTangentSpeed);

	void SetMaxRelativeTangentSpeed(FAGX_Real InMaxRelativeTangentSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRelativeTangentSpeed_BP() const;

	FAGX_Real GetMaxRelativeTangentSpeed() const;

	/**
	 * Maximum (rolling) speed for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRollingSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRollingSpeed_BP(float InMaxRollingSpeed);

	void SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRollingSpeed_BP() const;

	FAGX_Real GetMaxRollingSpeed() const;

	/**
	 * Adhesive force in the normal directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real NormalAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetNormalAdhesion_BP(float InNormalAdhesion);

	void SetNormalAdhesion(FAGX_Real InNormalAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetNormalAdhesion_BP() const;

	FAGX_Real GetNormalAdhesion() const;

	/**
	 * Adhesive force in the tangential directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real TangentialAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetTangentialAdhesion_BP(float InTangentialAdhesion);

	void SetTangentialAdhesion(FAGX_Real InTangentialAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetTangentialAdhesion_BP() const;

	FAGX_Real GetTangentialAdhesion() const;

	/**
	 * Check split given external forces for all objects merged (i.e., rb->getForce() the sum of
	 * rb->addForce(), including the gravity force).
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bMaySplitInGravityField {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaySplitInGravityField(bool bInMaySplitInGravityField);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	bool GetMaySplitInGravityField() const;

	/**
	 * True to split when Shape Contact state is agxCollide::GeometryContact::IMPACT_STATE, i.e.,
	 * the first time the objects collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bSplitOnLogicalImpact {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	bool GetSplitOnLogicalImpact() const;

	void CreateNative(UWorld* PlayingWorld);
	bool HasNative() const;
	FShapeContactMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	static UAGX_ShapeContactMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholds& Source);

	UAGX_ShapeContactMergeSplitThresholds* GetOrCreateInstance(UWorld* PlayingWorld);

	bool IsInstance() const;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CopyProperties(UAGX_ShapeContactMergeSplitThresholds& Source);
	void UpdateNativeProperties();

	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholds> Instance;
	TUniquePtr<FShapeContactMergeSplitThresholdsBarrier> NativeBarrier;
};