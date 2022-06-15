// Author: VMC Motion Technologies Co., Ltd.


#include "Vehicle/AGX_TrackRenderer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Vehicle/AGX_TrackComponent.h"

// Standard library includes.
#include <algorithm>

//#define TRACK_RENDERER_DETAILED_LOGGING

#define DEFAULT_VISUAL_SCALE FVector::OneVector
#define DEFAULT_VISUAL_OFFSET FVector::ZeroVector

namespace
{
	template<class T>
	T* FindFirstParentComponentByClass(USceneComponent* CurrentComponent)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const USceneComponent>::Value,
			"'T' template parameter to FindFirstParentComponentByClass must be derived from USceneComponent");

		if (CurrentComponent == nullptr)
		{
			return nullptr;
		}
		else if (CurrentComponent->IsA(T::StaticClass()))
		{
			return Cast<T>(CurrentComponent);
		}
		else
		{
			return FindFirstParentComponentByClass<T>(CurrentComponent->GetAttachParent());
		}
	}
}

UAGX_TrackRenderer::UAGX_TrackRenderer()
{
	// Set this component to be ticked every frame so that it can synchronize
	// the visual track node instance transforms.
	PrimaryComponentTick.bCanEverTick = true;

	// \todo We want to synchronize visuals AFTER the physics have stepped. Find correct group!
	//PrimaryComponentTick.TickGroup = ??;

	// Sets default values.
	Scale = DEFAULT_VISUAL_SCALE;
	Offset = DEFAULT_VISUAL_OFFSET;
	bAutoScaleAndOffset = true;
	LocalMeshBoundsMax = FVector::OneVector * 50.0f;
	LocalMeshBoundsMin = -FVector::OneVector * 50.0f;

	// Set default values in inherited classes.
	// Make sure Unreal's default physics collision is disabled.
	bDisableCollision = false;
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UAGX_TrackRenderer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// \todo We do not need to sync visual transforms if physics simulation have not been
	//       stepped since last time we synchronized.
	SynchronizeVisuals();
}

TStructOnScope<FActorComponentInstanceData> UAGX_TrackRenderer::GetComponentInstanceData() const
{
#ifdef TRACK_RENDERER_DETAILED_LOGGING
	UE_LOG(LogAGX, Verbose,
		TEXT("UAGX_TrackRenderer::GetComponentInstanceData() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	return Super::GetComponentInstanceData();
}

void UAGX_TrackRenderer::ApplyComponentInstanceData(
	struct FInstancedStaticMeshComponentInstanceData* ComponentInstanceData)
{
	Super::ApplyComponentInstanceData(ComponentInstanceData);

#ifdef TRACK_RENDERER_DETAILED_LOGGING
	UE_LOG(LogAGX, Verbose,
		TEXT("UAGX_TrackRenderer::ApplyComponentInstanceData() for '%s' (UID: %i) in '%s'. Phase unknown."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	// \note Actually only want to execute the code below for second phase, after properties have
	//       been fully deserialized, but because we need to use the parent class' component instance
	//       data we have no way ourselves to pass the CacheApplyPhase to this function.
	//if (CacheApplyPhase == ECacheApplyPhase::PostUserConstructionScript)
	{
		bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
		if (bIsPlaying)
		{
			// In case the game is paused, update the render data here, so we can see visual changes
			// immediately even if there is no tick yet.
			SynchronizeVisuals();
		}
		else
		{
			// Rebind and synchronize after second phase of BP actor instance reconstruction, so that
			// the visualization includes the property changes that caused the reconstruction.
			RebindToTrackPreviewNeedsUpdateEvent();
		}
	}
}

void UAGX_TrackRenderer::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR

void UAGX_TrackRenderer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
#ifdef TRACK_RENDERER_DETAILED_LOGGING
	UE_LOG(LogAGX, Verbose,
		TEXT("UAGX_TrackRenderer::PostEditChangeProperty() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	// Update the render data both regardless of playing or not.
	// If not playing, we do no only update render data when something changes, so do it here.
	// If playing, we update it every tick, so not strictly necessary here, but in the scenario
	// were the user has paused the simulation and is fine tuning the visual scale and offset,
	// we want to react to those changing and update the render data directly here (since the
	// next tick doesn't happen until game is un-paused).
	SynchronizeVisuals();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UAGX_TrackRenderer::PostLoad()
{
	Super::PostLoad();

	// Remove all instances in case they were saved during edit time.
	if (GetInstanceCount() > 0)
	{
		UE_LOG(LogAGX, Verbose,
			TEXT("'%s' in '%s' is removing %i instances during PostLoad()."),
			*GetName(), *GetNameSafe(GetOwner()), GetInstanceCount());

		ClearInstances();
	}

	bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// Bind to TrackPreviewNeedsUpdateEvent of the target track so that we know
		// when to update the track preview rendering while not playing.
		RebindToTrackPreviewNeedsUpdateEvent();
	}
}

void UAGX_TrackRenderer::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

#ifdef TRACK_RENDERER_DETAILED_LOGGING
	UE_LOG(LogAGX, Verbose,
		TEXT("UAGX_TrackRenderer::OnAttachmentChanged() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// Target track might have changed, so rebind to the TrackPreviewNeedsUpdateEvent
		// of the potentially new target track.
		RebindToTrackPreviewNeedsUpdateEvent();
	}
}

void UAGX_TrackRenderer::RebindToTrackPreviewNeedsUpdateEvent(bool bSynchronizeImmediately)
{
	if (IsBeingDestroyed())
	{
		return;
	}

	bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// Track preview only relevant when not playing.
		return;
	}

#ifdef TRACK_RENDERER_DETAILED_LOGGING
	UE_LOG(LogAGX, Verbose,
		TEXT("'%s' (UID: %i) in '%s' is rebinding TrackPreviewNeedsUpdateEvent."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	// Remove previous event binding, in case target track has changed.
	// \todo Do this in a more performance friendly way. For example we could keep a
	//       TWeakObjectPtr to the track whose event we had previously bound to, and
	//       only unbind from that one if it's different from current target track?
	for (TObjectIterator<UAGX_TrackComponent> It; It; ++It)
	{
		if (It->GetWorld() == GetWorld())
		{
			It->GetTrackPreviewNeedsUpdateEvent().RemoveAll(this);
		}
	}

	if (UAGX_TrackComponent* Track = FindTargetTrack())
	{

#ifdef TRACK_RENDERER_DETAILED_LOGGING
		UE_LOG(LogAGX, Verbose,
			TEXT("'%s' (UID: %i) in '%s' is registering to TrackPreviewNeedsUpdateEvent "
				"of '%s' (UID: %i) in '%s'"),
			*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()),
			*Track->GetName(), Track->GetUniqueID(), *GetNameSafe(Track->GetOwner()));
#endif

		// Bind to event in target track that lets us know when the Track Preview Data
		// needs an update.
		TWeakObjectPtr<ThisClass> SelfWeakPtr(this);
		Track->GetTrackPreviewNeedsUpdateEvent().AddLambda(
			[SelfWeakPtr](UAGX_TrackComponent* Source)
			{
				if (ThisClass* SelfPtr = SelfWeakPtr.Get())
				{
					// Make sure target track fired the event. Normally the if-condition shouldn't
					// evaluate to false, but it might if the target track has changed and we have
					// missed unbinding from the previous track's event. In that case, unbind now.
					if (Source == SelfPtr->FindTargetTrack())
					{
						SelfPtr->SynchronizeVisuals();
					}
					else
					{
						Source->GetTrackPreviewNeedsUpdateEvent().RemoveAll(SelfPtr);
					}
				}
			});
	}

	if (bSynchronizeImmediately)
	{
		SynchronizeVisuals();
	}
}

UAGX_TrackComponent* UAGX_TrackRenderer::FindTargetTrack()
{
	return FindFirstParentComponentByClass<UAGX_TrackComponent>(this); // \todo Cache component!
}

void UAGX_TrackRenderer::SetInstanceCount(int32 Count)
{
	Count = std::max(0, Count);

	while (GetInstanceCount() < Count)
	{
		AddInstance(FTransform());
	}
	while (GetInstanceCount() > Count)
	{
		RemoveInstance(GetInstanceCount() - 1);
	}
}

void UAGX_TrackRenderer::SynchronizeVisuals()
{
	UAGX_TrackComponent* Track = FindTargetTrack();

#ifdef TRACK_RENDERER_DETAILED_LOGGING
	if (!IsValid(Track))
	{
		// \note This warning can happen during BP instance reconstruction when SynchronizeVisuals is
		//       called prematurely (i.e. before all properties have been fully initialized both on the
		//       the renderer and the track component), but is no problem if it is called again later
		//       during the reconstruction when all properties have been fully deserialized.
		UE_LOG(LogAGX, Warning,
			TEXT("'%s' (UID: %i) in '%s' is synchronizing visuals but no valid track was found."),
			*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
	}
#endif

	if (IsBeingDestroyed())
	{
		return;
	}

	// Get the mesh instance transforms, either from the native if playing or
	// from the preview data if not playing.
	if (!ComputeNodeTransforms(NodeTransformsCache, Track))
	{
		NodeTransformsCache.Empty(); // if failed, do not render anything.
	}

	// Make sure there is one mesh instance per track node.
	const int32 NumNodes = NodeTransformsCache.Num();
	SetInstanceCount(NumNodes);

	// Because UInstancedStaticMeshComponent::UpdateInstanceTransform() converts instance transforms
	// from World to Local Transform Space, make sure our local transform space is up-to-date.
	UpdateComponentToWorld();

	// Update transforms of the track node mesh instances.
	for (int32 i = 0; i < NumNodes; ++i)
	{
		UpdateInstanceTransform(i, NodeTransformsCache[i], /*bWorldSpace*/ true);
	}
}

bool UAGX_TrackRenderer::ComputeNodeTransforms(TArray<FTransform>& OutTransforms, UAGX_TrackComponent* Track)
{
	if (!IsValid(Track))
		return false;

	// Get node transforms either from the actual track when playing,
	// or from a generated preview if not playing.
	bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// Get mesh instance transforms from the native.

		if (!Track->HasNative())
		{
			return false;
		}

		FVector VisualScale, VisualOffset;
		if (bAutoScaleAndOffset)
		{
			ComputeVisualScaleAndOffset(VisualScale, VisualOffset, Track->GetNative()->GetNodeSize(0));
		}
		else
		{
			VisualScale = Scale;
			VisualOffset = Offset;
		}

		Track->GetNative()->GetNodeTransforms(OutTransforms, VisualScale, VisualOffset);
	}
	else
	{
		// Get mesh instance transforms from preview data.

		FAGX_TrackPreviewData* Preview = Track->GetTrackPreview(
			/*bUpdateIfNecessary*/ true, /*bForceUpdate*/ false);

		if (!Preview || Preview->NodeTransforms.Num() <= 0)
		{
			UE_LOG(LogAGX, Error, TEXT("Failed to generate Track Preview Data."));
			return false;
		}
		check(Preview->NodeTransforms.Num() == Preview->NodeHalfExtents.Num());

		FVector PhysicsNodeSize = 2 * Preview->NodeHalfExtents[0];
		FVector BodyFrameToNodeCenter = FVector(0, 0, 0.5f * PhysicsNodeSize.Z);
		FVector VisualScale, VisualOffset;
		if (bAutoScaleAndOffset)
		{
			ComputeVisualScaleAndOffset(VisualScale, VisualOffset, PhysicsNodeSize);
		}
		else
		{
			VisualScale = Scale;
			VisualOffset = Offset;
		}

		OutTransforms.SetNum(Preview->NodeTransforms.Num(), /*bAllowShrinking*/ true);
		for (int i = 0; i < Preview->NodeTransforms.Num(); ++i)
		{
			FVector WorldOffset = Preview->NodeTransforms[i].GetRotation().RotateVector(
				VisualOffset + BodyFrameToNodeCenter);

			OutTransforms[i].SetScale3D(VisualScale);
			OutTransforms[i].SetRotation(Preview->NodeTransforms[i].GetRotation());
			OutTransforms[i].SetLocation(Preview->NodeTransforms[i].GetTranslation() + WorldOffset);
		}
	}
	return true;
}

bool UAGX_TrackRenderer::ComputeVisualScaleAndOffset(
	FVector& OutVisualScale, FVector& OutVisualOffset, const FVector& PhysicsNodeSize) const
{
	FVector LocalMeshBoundsSize = LocalMeshBoundsMax - LocalMeshBoundsMin;
	FVector LocalBoundsCenter = LocalMeshBoundsMin + LocalMeshBoundsSize * 0.5f;

	if (FMath::IsNearlyZero(LocalMeshBoundsSize.X) ||
		FMath::IsNearlyZero(LocalMeshBoundsSize.Y) ||
		FMath::IsNearlyZero(LocalMeshBoundsSize.Z))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to compute visual Scale and Offset for '%s' in '%s' because LocalMeshBoundsMax is too close "
				"too LocalMeshBoundsMin."),
			*GetName(), *GetNameSafe(GetOwner()));
		OutVisualScale = DEFAULT_VISUAL_SCALE;
		OutVisualOffset = DEFAULT_VISUAL_OFFSET;
		return false;
	}

	OutVisualScale = PhysicsNodeSize / LocalMeshBoundsSize;
	OutVisualOffset = -LocalBoundsCenter * Scale;  // times scale to convert offset to post-scale coordinates
	return true;
}

#undef DEFAULT_VISUAL_SCALE
#undef DEFAULT_VISUAL_OFFSET
