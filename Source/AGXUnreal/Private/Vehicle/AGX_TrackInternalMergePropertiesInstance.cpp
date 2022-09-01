// Copyright 2022, Algoryx Simulation AB.


#include "Vehicle/AGX_TrackInternalMergePropertiesInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackInternalMergePropertiesAsset.h"

// AGX Dynamics for Unreal Barrier includes.
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"


UAGX_TrackInternalMergePropertiesInstance* UAGX_TrackInternalMergePropertiesInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TrackInternalMergePropertiesAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TrackInternalMergePropertiesInstance* NewInstance = NewObject<UAGX_TrackInternalMergePropertiesInstance>(
		Outer, UAGX_TrackInternalMergePropertiesInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyFrom(Source);
	NewInstance->SourceAsset = Source;

	return NewInstance;
}

UAGX_TrackInternalMergePropertiesInstance::~UAGX_TrackInternalMergePropertiesInstance()
{
}

UAGX_TrackInternalMergePropertiesAsset* UAGX_TrackInternalMergePropertiesInstance::GetAsset()
{
	return SourceAsset.Get();
}

void UAGX_TrackInternalMergePropertiesInstance::RegisterTargetTrack(UAGX_TrackComponent* Track)
{
	TargetTracks.AddUnique(Track);

	// \todo Remove no longer valid TargetTargets? (i.e. tracks that has become null, for example
	//       due to BP instance reconstruction, or tracks that do no longer use this asset).

	UpdateNativePropertiesOnTrack(Track);
}

void UAGX_TrackInternalMergePropertiesInstance::UnregisterTargetTrack(UAGX_TrackComponent* Track)
{
	TargetTracks.Remove(Track);
}

void UAGX_TrackInternalMergePropertiesInstance::UpdateNativePropertiesOnAllTargetTracks()
{
	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		UpdateNativePropertiesOnTrack(Track.Get());
	}
}

void UAGX_TrackInternalMergePropertiesInstance::UpdateNativePropertiesOnTrack(UAGX_TrackComponent* Track)
{
	if (!IsValidTarget(Track))
		return;

	// \note Be aware that this method is probably be called from UAGX_TrackComponent::CreateNative(),
	//       which means that the native is potentially under construction and that it is therefore
	//       a bit sensitive to exactly where from within that function that this function is called.

	FTrackBarrier* TrackBarrier = Track->GetOrCreateNative();
	if (TrackBarrier == nullptr || !TrackBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to update native properties of TrackInternalMergeProperties '%s' on "
				"Track '%s' in '%s' because a valid native could not be found."),
				*GetName(), *Track->GetName(), *GetLabelSafe(Track->GetOwner()));
		return;
	}
	check(TrackBarrier->HasNative());

	TrackBarrier->InternalMergeProperties_SetEnableMerge(bMergeEnabled);
	TrackBarrier->InternalMergeProperties_SetNumNodesPerMergeSegment(NumNodesPerMergeSegment);
	TrackBarrier->InternalMergeProperties_SetContactReduction(static_cast<uint8>(ContactReduction));
	TrackBarrier->InternalMergeProperties_SetEnableLockToReachMergeCondition(bLockToReachMergeConditionEnabled);
	TrackBarrier->InternalMergeProperties_SetLockToReachMergeConditionCompliance(LockToReachMergeConditionCompliance);
	TrackBarrier->InternalMergeProperties_SetLockToReachMergeConditionDamping(LockToReachMergeConditionDamping);
	TrackBarrier->InternalMergeProperties_SetMaxAngleMergeCondition(MaxAngleMergeCondition);
}

void UAGX_TrackInternalMergePropertiesInstance::SetMergeEnabled(bool bEnabled)
{
	Super::SetMergeEnabled(bEnabled);

	// \todo This loop syntax is repeated in all SetXXX() functions. Find a way to refactor it.
	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetEnableMerge(
				this->bMergeEnabled);
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetNumNodesPerMergeSegment(int InNumNodesPerMergeSegment)
{
	Super::SetNumNodesPerMergeSegment(InNumNodesPerMergeSegment);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetNumNodesPerMergeSegment(
				this->NumNodesPerMergeSegment);
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetContactReduction(
	EAGX_MergedTrackNodeContactReduction InContactReduction)
{
	Super::SetContactReduction(InContactReduction);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetContactReduction(
				static_cast<uint8>(this->ContactReduction));
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetLockToReachMergeConditionEnabled(bool bEnabled)
{
	Super::SetLockToReachMergeConditionEnabled(bEnabled);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetEnableLockToReachMergeCondition(
				this->bLockToReachMergeConditionEnabled);
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetLockToReachMergeConditionCompliance(double Compliance)
{
	Super::SetLockToReachMergeConditionCompliance(Compliance);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetLockToReachMergeConditionCompliance(
				this->LockToReachMergeConditionCompliance);
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetLockToReachMergeConditionDamping(double Damping)
{
	Super::SetLockToReachMergeConditionDamping(Damping);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetLockToReachMergeConditionDamping(
				this->LockToReachMergeConditionDamping);
		}
	}
}

void UAGX_TrackInternalMergePropertiesInstance::SetMaxAngleMergeCondition(double MaxAngleToMerge)
{
	Super::SetMaxAngleMergeCondition(MaxAngleToMerge);

	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		if (Track.IsValid() && IsValidTarget(Track.Get()))
		{
			Track->GetNative()->InternalMergeProperties_SetMaxAngleMergeCondition(
				this->MaxAngleMergeCondition);
		}
	}
}

UAGX_TrackInternalMergePropertiesInstance* UAGX_TrackInternalMergePropertiesInstance::GetInstance()
{
	return this;
}

UAGX_TrackInternalMergePropertiesInstance* UAGX_TrackInternalMergePropertiesInstance::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	return this;
}

bool UAGX_TrackInternalMergePropertiesInstance::IsValidTarget(UAGX_TrackComponent* Track)
{
	return
		IsValid(Track) &&
		Track->HasNative() &&
		Track->InternalMergeProperties != nullptr &&
		Track->InternalMergeProperties->GetInstance() == this;
}
