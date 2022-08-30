// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholdsBase.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholdsBase : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UAGX_ShapeContactMergeSplitThresholdsBase() = default;

	virtual UAGX_ShapeContactMergeSplitThresholdsBase* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_ShapeContactMergeSplitThresholdsBase::GetOrCreateInstance,
					 return nullptr;);

	/**
	 * Maximum impact speed (along a contact normal) a merged object can resist
	 * without being split [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxImpactSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxImpactSpeed_AsFloat(float InMaxImpactSpeed);
	virtual void SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxImpactSpeed_AsFloat() const;
	virtual FAGX_Real GetMaxImpactSpeed() const;

	/**
	 * Maximum speed along a contact normal for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeNormalSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeNormalSpeed_AsFloat(float InMaxRelativeNormalSpeed);
	virtual void SetMaxRelativeNormalSpeed(FAGX_Real InMaxRelativeNormalSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRelativeNormalSpeed_AsFloat() const;
	virtual FAGX_Real GetMaxRelativeNormalSpeed() const;

	/**
	 * Maximum (sliding) speed along a contact tangent for a contact to be considered resting
	 * [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeTangentSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeTangentSpeed_AsFloat(float InMaxRelativeTangentSpeed);
	virtual void SetMaxRelativeTangentSpeed(FAGX_Real InMaxRelativeTangentSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRelativeTangentSpeed_AsFloat() const;
	virtual FAGX_Real GetMaxRelativeTangentSpeed() const;

	/**
	 * Maximum (rolling) speed for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRollingSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRollingSpeed_AsFloat(float InMaxRollingSpeed);
	virtual void SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetMaxRollingSpeed_AsFloat() const;
	virtual FAGX_Real GetMaxRollingSpeed() const;

	/**
	 * Adhesive force in the normal directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real NormalAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetNormalAdhesion_AsFloat(float InNormalAdhesion);
	virtual void SetNormalAdhesion(FAGX_Real InNormalAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetNormalAdhesion_AsFloat() const;
	virtual FAGX_Real GetNormalAdhesion() const;

	/**
	 * Adhesive force in the tangential directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real TangentialAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetTangentialAdhesion_AsFloat(float InTangentialAdhesion);
	virtual void SetTangentialAdhesion(FAGX_Real InTangentialAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	float GetTangentialAdhesion_AsFloat() const;
	virtual FAGX_Real GetTangentialAdhesion() const;

	/**
	 * Check split given external forces for all objects merged (i.e., rb->getForce() the sum of
	 * rb->addForce(), including the gravity force).
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bMaySplitInGravityField {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	virtual void SetMaySplitInGravityField(bool bInMaySplitInGravityField);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	virtual bool GetMaySplitInGravityField() const;

	/**
	 * True to split when Shape Contact state is agxCollide::GeometryContact::IMPACT_STATE, i.e.,
	 * the first time the objects collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bSplitOnLogicalImpact {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	virtual void SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	virtual bool GetSplitOnLogicalImpact() const;
};