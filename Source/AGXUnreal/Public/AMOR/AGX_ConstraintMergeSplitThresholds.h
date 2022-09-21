// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/AGX_MergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholds.generated.h"

/*
 * Defines the thresholds used by AMOR (merge split) for Constraints, affecting under which
 * conditions Constraints will merge and split.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholds : public UAGX_MergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	/**
	 * Maximum difference the 'force range'/desired force range parameter in any controller
	 * may change without splitting the constrained objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredForceRangeDiff {0.1};

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Set Max Desired Force Range Diff"))
	void SetMaxDesiredForceRangeDiff_BP(float InMaxDesiredForceRangeDiff);

	void SetMaxDesiredForceRangeDiff(double InMaxDesiredForceRangeDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Get Max Desired Force Range Diff"))
	float GetMaxDesiredForceRangeDiff_BP() const;

	double GetMaxDesiredForceRangeDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a lock controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredLockAngleDiff {0.00001};

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Set Max Desired Lock Angle Diff"))
	void SetMaxDesiredLockAngleDiff_BP(float InMaxDesiredLockAngleDiff);

	void SetMaxDesiredLockAngleDiff(double InMaxDesiredLockAngleDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Get Max Desired Lock Angle Diff"))
	float GetMaxDesiredLockAngleDiff_BP() const;

	double GetMaxDesiredLockAngleDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a range controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredRangeAngleDiff {0.00001};

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Set Max Desired Range Angle Diff"))
	void SetMaxDesiredRangeAngleDiff_BP(float InMaxDesiredRangeAngleDiff);

	void SetMaxDesiredRangeAngleDiff(double InMaxDesiredRangeAngleDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Get Max Desired Range Angle Diff"))
	float GetMaxDesiredRangeAngleDiff_BP() const;

	double GetMaxDesiredRangeAngleDiff() const;

	/**
	 * Maximum difference the 'speed'/desired speed parameter in a motor controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredSpeedDiff {0.00001};

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Set Max Desired Speed Diff"))
	void SetMaxDesiredSpeedDiff_BP(float InMaxDesiredSpeedDiff);

	void SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Get MaxDesired Speed Diff"))
	float GetMaxDesiredSpeedDiff_BP() const;

	double GetMaxDesiredSpeedDiff() const;

	/**
	 * Maximum relative speed between the constrained objects for the system to be considered at
	 * rest.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxRelativeSpeed {0.005};

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Set Max Relative Speed"))
	void SetMaxRelativeSpeed_BP(float InMaxRelativeSpeed);

	void SetMaxRelativeSpeed(double InMaxRelativeSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta = (DisplayName = "Get Max Relative Speed"))
	float GetMaxRelativeSpeed_BP() const;

	double GetMaxRelativeSpeed() const;

	void CreateNative(UWorld* PlayingWorld, bool bIsRotational);
	bool HasNative() const;
	FConstraintMergeSplitThresholdsBarrier* GetOrCreateNative(
		UWorld* PlayingWorld, bool bIsRotational);
	FConstraintMergeSplitThresholdsBarrier* GetNative();

	static UAGX_ConstraintMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholds& Source, bool bIsRotational);

	UAGX_ConstraintMergeSplitThresholds* GetOrCreateInstance(
		UWorld* PlayingWorld, bool bIsRotational);

	UAGX_ConstraintMergeSplitThresholds* GetInstance();

	bool IsInstance() const;

	void CopyFrom(const FMergeSplitThresholdsBarrier& Barrier);

	/*
	 * Assigns the property values of this class to the passed barrier.
	 */
	void CopyTo(FConstraintMergeSplitThresholdsBarrier& Barrier);

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void SetNativeProperties();
	void CopyFrom(const UAGX_ConstraintMergeSplitThresholds& Source);

	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Instance;
	FConstraintMergeSplitThresholdsBarrier NativeBarrier;
};