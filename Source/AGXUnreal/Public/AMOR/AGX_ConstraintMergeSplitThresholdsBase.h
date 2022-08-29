// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholdsBase.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholdsBase : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UAGX_ConstraintMergeSplitThresholdsBase() = default;

	virtual UAGX_ConstraintMergeSplitThresholdsBase* GetOrCreateInstance(
		UWorld* PlayingWorld, bool bIsRotational)
		PURE_VIRTUAL(UAGX_ConstraintMergeSplitThresholdsBase::GetOrCreateInstance,
					 return nullptr;);

	/**
	 * Maximum difference the 'force range'/desired force range parameter in any controller
	 * may change without splitting the constrained objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredForceRangeDiff;

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredForceRangeDiff_AsFloat(float InMaxDesiredForceRangeDiff);
	virtual void SetMaxDesiredForceRangeDiff(FAGX_Real InMaxDesiredForceRangeDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredForceRangeDiff_AsFloat() const;
	virtual FAGX_Real GetMaxDesiredForceRangeDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a lock controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredLockAngleDiff;

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredLockAngleDiff_AsFloat(float InMaxDesiredLockAngleDiff);
	virtual void SetMaxDesiredLockAngleDiff(FAGX_Real InMaxDesiredLockAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredLockAngleDiff_AsFloat() const;
	virtual FAGX_Real GetMaxDesiredLockAngleDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a range controller may
	 * change without splitting the constrained objects. 
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredRangeAngleDiff;

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredRangeAngleDiff_AsFloat(float InMaxDesiredRangeAngleDiff);
	virtual void SetMaxDesiredRangeAngleDiff(FAGX_Real InMaxDesiredRangeAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredRangeAngleDiff_AsFloat() const;
	virtual FAGX_Real GetMaxDesiredRangeAngleDiff() const;

	/**
	 * Maximum difference the 'speed'/desired speed parameter in a motor controller may
	 * change without splitting the constrained objects. 
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredSpeedDiff;

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredSpeedDiff_AsFloat(float InMaxDesiredSpeedDiff);
	virtual void SetMaxDesiredSpeedDiff(FAGX_Real InMaxDesiredSpeedDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredSpeedDiff_AsFloat() const;
	virtual FAGX_Real GetMaxDesiredSpeedDiff() const;

	/**
	 * Maximum relative speed between the constrained objects for the system to be considered at
	 * rest.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxRelativeSpeed;

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxRelativeSpeed_AsFloat(float InMaxRelativeSpeed);
	virtual void SetMaxRelativeSpeed(FAGX_Real InMaxRelativeSpeed);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxRelativeSpeed_AsFloat() const;
	virtual FAGX_Real GetMaxRelativeSpeed() const;
};