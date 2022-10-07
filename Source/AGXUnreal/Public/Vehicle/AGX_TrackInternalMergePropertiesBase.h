// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ContactMaterialReductionMode.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackInternalMergePropertiesBase.generated.h"

class FTrackBarrier;
class UAGX_TrackInternalMergePropertiesAsset;
class UAGX_TrackInternalMergePropertiesInstance;

/**
 * Contact reduction of merged nodes in contact with other objects such as ground.
 */
UENUM(BlueprintType)
enum class EAGX_MergedTrackNodeContactReduction : uint8
{
	/** Contact reduction disabled. */
	None,

	/** Contact reduction enabled with bin resolution = 3. */
	Minimal,

	/** Contact reduction enabled with bin resolution = 2. */
	Moderate,

	/** Contact reduction enabled with bin resolution = 1. */
	Aggressive
};

/**
 * Properties and thresholds for internal merging of nodes in AGX Track Component.
 *
 * Does not own any AGX Native. Instead, it applies its data to the native Track of each
 * Track Component that uses this asset. This means that modifications done directly on a Track
 * will overwrite the state set by a Track Internal Merge Properties but it will not override
 * it, meaning that setting the property on the Track Internal Merge Properties again will overwrite
 * the value set on the Track directly. Reading will always read the Track Internal Merge Properties
 * value, which may be different from the Tracks' value(s) if it has been overwritten on a
 * particular Track.
 *
 * Base class. Inherited by the -Asset subclass, which represents assets created in the Content
 * Browser, and the -Instance subclass, which represents an instance of the asset and exists during
 * a single Play session only.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable, abstract)
class AGXUNREAL_API UAGX_TrackInternalMergePropertiesBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Whether to enable merging of internal nodes into segments in the track.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Internal Merge Properties")
	bool bMergeEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetMergeEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual bool GetMergeEnabled() const;

	/**
	 * Maximum number of consecutive nodes that may be merged together into a segment.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled", ClampMin = "1"))
	int32 NumNodesPerMergeSegment;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetNumNodesPerMergeSegment(int32 InNumNodesPerMergeSegment);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual int32 GetNumNodesPerMergeSegment() const;

	/**
	 * Contact reduction level of merged nodes against other objects, such as ground.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	EAGX_MergedTrackNodeContactReduction ContactReduction;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetContactReduction(EAGX_MergedTrackNodeContactReduction InContactReduction);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual EAGX_MergedTrackNodeContactReduction GetContactReduction() const;

	/**
	 * Whether to enable the usage of hinge lock to reach merge condition, i.e. angle close to zero.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	bool bLockToReachMergeConditionEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetLockToReachMergeConditionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual bool GetLockToReachMergeConditionEnabled() const;

	/**
	 * Compliance of the hinge lock used to reach merge condition. [rad/Nm]
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionCompliance;

	virtual double GetLockToReachMergeConditionCompliance() const;

	virtual void SetLockToReachMergeConditionCompliance(double Compliance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Get Lock To Reach Merge Condition Compliance"))
	float GetLockToReachMergeConditionCompliance_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Lock To Reach Merge Condition Compliance"))
	virtual void SetLockToReachMergeConditionCompliance_BP(float Compliance);

	/**
	 * Damping of the hinge lock used to reach merge condition. [s]
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionDamping;

	virtual double GetLockToReachMergeConditionDamping() const;

	virtual void SetLockToReachMergeConditionDamping(double Damping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DislayName = "Get Lock To Reach Merge Condition Damping"))
	virtual float GetLockToReachMergeConditionDamping_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Lock To Reach Merge Condition Damping"))
	virtual void SetLockToReachMergeConditionDamping_BP(float Damping);

	/**
	 * Maximum angle to trigger merge between nodes. [degrees]
	 *
	 * I.e., when the angle between two nodes < maxAngleToMerge the nodes will merge.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	FAGX_Real MaxAngleMergeCondition;

	virtual void SetMaxAngleMergeCondition(double MaxAngleToMerge);

	virtual double GetMaxAngleMergeCondition() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Get Max Angle Merge Condition"))
	virtual float GetMaxAngleMergeCondition_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Max Angle Merge Condition"))
	virtual void SetMaxAngleMergeCondition_BP(float MaxAngleToMerge);

public:
	UAGX_TrackInternalMergePropertiesBase();

	virtual ~UAGX_TrackInternalMergePropertiesBase();

	static EAGX_MergedTrackNodeContactReduction ToContactReduction(uint8 ContactReduction);

	virtual UAGX_TrackInternalMergePropertiesInstance* GetInstance()
		PURE_VIRTUAL(UAGX_TrackInternalMergePropertiesBase::GetInstance, return nullptr;);

	/**
	 * If PlayingWorld is an in-game World and this TrackInternalMergeProperties is a
	 * UAGX_TrackInternalMergePropertiesAsset, returns a UAGX_TrackInternalMergePropertiesInstance
	 * representing the TrackInternalMergeProperties asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_TrackInternalMergePropertiesInstance it returns
	 * itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_TrackInternalMergePropertiesInstance* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_TrackInternalMergePropertiesBase::GetOrCreateInstance, return nullptr;);

	/**
	 * If this TrackInternalMergeProperties is a UAGX_TrackInternalMergePropertiesInstance, returns
	 * the UAGX_TrackInternalMergePropertiesAsset it was created from (if it still exists). Else
	 * returns null.
	 */
	virtual UAGX_TrackInternalMergePropertiesAsset* GetAsset()
		PURE_VIRTUAL(UAGX_TrackInternalMergePropertiesBase::GetAsset, return nullptr;);

	void CopyFrom(const UAGX_TrackInternalMergePropertiesBase* Source);
	void CopyFrom(const FTrackBarrier& Barrier);

#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every UPROPERTY in PostEditChangeProperty and PostEditChangeChainProperty.
	virtual void InitPropertyDispatcher();
#endif

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.
};
