// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergePropertiesBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackInternalMergePropertiesInstance.generated.h"

class UAGX_TrackComponent;
class UAGX_TrackInternalMergePropertiesAsset;

/**
 * Represents a native AGX TrackInternalMergeProperties in-game. Should never exist when not playing.
 *
 * Should only be created using the static function CreateFromAsset, which copyies data from its
 * sibling class UAGX_TrackInternalMergePropertiesAsset.
 */
UCLASS()
class AGXUNREAL_API UAGX_TrackInternalMergePropertiesInstance : public UAGX_TrackInternalMergePropertiesBase
{
	GENERATED_BODY()

public:
	static UAGX_TrackInternalMergePropertiesInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TrackInternalMergePropertiesAsset* Source);

public:
	virtual ~UAGX_TrackInternalMergePropertiesInstance();

	virtual UAGX_TrackInternalMergePropertiesAsset* GetAsset() override;

	void RegisterTargetTrack(UAGX_TrackComponent* Track);
	void UnregisterTargetTrack(UAGX_TrackComponent* Track);

	void UpdateNativePropertiesOnAllTargetTracks();
	void UpdateNativePropertiesOnTrack(UAGX_TrackComponent* Track);

	// ~Begin UAGX_TrackInternalMergePropertiesBase interface.
	virtual void SetMergeEnabled(bool bEnabled) override;
	virtual void SetNumNodesPerMergeSegment(int InNumNodesPerMergeSegment) override;
	virtual void SetContactReduction(EAGX_MergedTrackNodeContactReduction InContactReduction) override;
	virtual void SetLockToReachMergeConditionEnabled(bool bEnabled) override;
	virtual void SetLockToReachMergeConditionCompliance(FAGX_Real Compliance) override;
	virtual void SetLockToReachMergeConditionDamping(FAGX_Real Damping) override;
	virtual void SetMaxAngleMergeCondition(FAGX_Real MaxAngleToMerge) override;
	virtual UAGX_TrackInternalMergePropertiesInstance* GetInstance() override;
	virtual UAGX_TrackInternalMergePropertiesInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
	// ~End UAGX_TrackInternalMergePropertiesBase interface.

private:

	// Return true if track is not null and its InternalMergeProperties point to this.
	bool IsValidTarget(UAGX_TrackComponent* Track);

	/// \todo This member is probably not necessary.. Remove it?
	TWeakObjectPtr<UAGX_TrackInternalMergePropertiesAsset> SourceAsset;

	// List of tracks that this InternalMergeProperties should write to.
	// \todo What happens if target track is in a BP Actor Instance that gets reconstructed?
	TArray<TWeakObjectPtr<UAGX_TrackComponent>> TargetTracks;
};
