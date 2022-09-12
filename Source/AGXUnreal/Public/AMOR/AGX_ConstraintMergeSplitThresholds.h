// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholds.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholds : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Maximum difference the 'force range'/desired force range parameter in any controller
	 * may change without splitting the constrained objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredForceRangeDiff {0.1};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredForceRangeDiff_BP(float InMaxDesiredForceRangeDiff);

	void SetMaxDesiredForceRangeDiff(FAGX_Real InMaxDesiredForceRangeDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredForceRangeDiff_BP() const;

	FAGX_Real GetMaxDesiredForceRangeDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a lock controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredLockAngleDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredLockAngleDiff_BP(float InMaxDesiredLockAngleDiff);

	void SetMaxDesiredLockAngleDiff(FAGX_Real InMaxDesiredLockAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredLockAngleDiff_BP() const;

	FAGX_Real GetMaxDesiredLockAngleDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a range controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredRangeAngleDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredRangeAngleDiff_BP(float InMaxDesiredRangeAngleDiff);

	void SetMaxDesiredRangeAngleDiff(FAGX_Real InMaxDesiredRangeAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredRangeAngleDiff_BP() const;

	FAGX_Real GetMaxDesiredRangeAngleDiff() const;

	/**
	 * Maximum difference the 'speed'/desired speed parameter in a motor controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredSpeedDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredSpeedDiff_BP(float InMaxDesiredSpeedDiff);

	void SetMaxDesiredSpeedDiff(FAGX_Real InMaxDesiredSpeedDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxDesiredSpeedDiff_BP() const;

	FAGX_Real GetMaxDesiredSpeedDiff() const;

	/**
	 * Maximum relative speed between the constrained objects for the system to be considered at
	 * rest.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxRelativeSpeed {0.005};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxRelativeSpeed_BP(float InMaxRelativeSpeed);

	void SetMaxRelativeSpeed(FAGX_Real InMaxRelativeSpeed);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	float GetMaxRelativeSpeed_BP() const;

	FAGX_Real GetMaxRelativeSpeed() const;

	void CreateNative(UWorld* PlayingWorld, bool bIsRotational);
	bool HasNative() const;
	FConstraintMergeSplitThresholdsBarrier* GetOrCreateNative(
		UWorld* PlayingWorld, bool bIsRotational);

	static UAGX_ConstraintMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholds& Source, bool bIsRotational);

	UAGX_ConstraintMergeSplitThresholds* GetOrCreateInstance(
		UWorld* PlayingWorld, bool bIsRotational);

	bool IsInstance() const;

	/*
	 * Assigns the property values of this class to the passed barrier.
	 */
	void SetNativeProperties(FConstraintMergeSplitThresholdsBarrier& Barrier);

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CopyProperties(UAGX_ConstraintMergeSplitThresholds& Source);

	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Instance;
	TUniquePtr<FConstraintMergeSplitThresholdsBarrier> NativeBarrier;
};