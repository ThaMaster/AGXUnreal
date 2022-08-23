// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ContactMaterialReductionMode.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackInternalMergePropertiesBase.generated.h"

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
 * Track Component that uses this asset.
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

	/**
	 * Maximum number of consecutive nodes that may merge together into a segment.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled", ClampMin = "1"))
	uint32 NumNodesPerMergeSegment;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetNumNodesPerMergeSegment(int InNumNodesPerMergeSegment);

	/**
	 * Contact reduction level of merged nodes against other objects.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	EAGX_MergedTrackNodeContactReduction ContactReduction;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetContactReduction(EAGX_MergedTrackNodeContactReduction InContactReduction);

	/**
	 * Whether to enable the usage of hinge lock to reach merge condition (angle close to zero).
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	bool bLockToReachMergeConditionEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	virtual void SetLockToReachMergeConditionEnabled(bool bEnabled);

	/**
	 * Compliance of the hinge lock used to reach merge condition.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionCompliance;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Lock To Reach Merge Condition Compliance"))
	void SetLockToReachMergeConditionCompliance_AsFloat(float Compliance);

	virtual void SetLockToReachMergeConditionCompliance(double Compliance);

	/**
	 * Damping of the hinge lock used to reach merge condition.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionDamping;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Lock To Reach Merge Condition Damping"))
	void SetLockToReachMergeConditionDamping_AsFloat(float Damping);

	virtual void SetLockToReachMergeConditionDamping(double Damping);

	/**
	 * Maximum angle to trigger merge between nodes. [degrees]
	 *
	 * I.e., when the angle between two nodes < maxAngleToMerge the nodes will merge.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	FAGX_Real MaxAngleMergeCondition;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DisplayName = "Set Max Angle Merge Condition"))
	void SetMaxAngleMergeCondition_AsFloat(float MaxAngleToMerge);

	virtual void SetMaxAngleMergeCondition(double MaxAngleToMerge);

public:
	/**
	 * Invokes the member function GetOrCreateInstance() on TrackInternalMergeProperties pointed to
	 * by Property, assigns the return value to Property, and then returns it. Returns null and does
	 * nothing if PlayingWorld is not an in-game world.
	 */
	static UAGX_TrackInternalMergePropertiesInstance* GetOrCreateInstance(
		UWorld* PlayingWorld, UAGX_TrackInternalMergePropertiesBase*& Property);

	UAGX_TrackInternalMergePropertiesBase();

	virtual ~UAGX_TrackInternalMergePropertiesBase();

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
