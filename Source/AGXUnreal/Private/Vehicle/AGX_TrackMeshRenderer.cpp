// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackMeshRenderer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"

// Unreal Engine includes.
#include "Materials/Material.h"
#include "Engine/World.h"

// Standard library includes.
#include <algorithm>

// #define TRACK_MESH_RENDERER_DETAILED_LOGGING

#define LOCTEXT_NAMESPACE "AGX_TrackMeshRenderer"

namespace AGX_TrackMeshRenderer_helpers
{
	template <class T>
	T* FindFirstParentComponentByClass(USceneComponent* CurrentComponent)
	{
		static_assert(
			TPointerIsConvertibleFromTo<T, const USceneComponent>::Value,
			"'T' template parameter to FindFirstParentComponentByClass must be derived from "
			"USceneComponent");

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

#if WITH_EDITOR
	void EnsureValidRenderMaterial(UAGX_TrackMeshRenderer& TrackMeshRenderer)
	{
		if (TrackMeshRenderer.OverrideMaterials.Num() == 0)
			return;

		for (auto& MatInterface : TrackMeshRenderer.OverrideMaterials)
		{
			if (MatInterface == nullptr)
				continue;

			UMaterial* Material = MatInterface->GetMaterial();
			if (Material == nullptr || Material->bUsedWithInstancedStaticMeshes)
				return;

			if (Material->GetPathName().StartsWith("/Game/"))
			{
				// This is a material part of the UE project itself. We can therefore be a bit more
				// helpful and offer to fix the material setting and save the asset so that the user
				// does not have to manually do it.
				const FText AskEnableUseWithInstancedSM = LOCTEXT(
					"EnableUseWithInstancedStaticMeshes?",
					"The selected Material does not have Use With Instanced Static Meshes enabled, "
					"meaning that it cannot be used with the Track Mesh Renderer. Would you like "
					"this "
					"setting to be automatically enabled? The Material asset will be re-saved.");
				if (FAGX_NotificationUtilities::YesNoQuestion(AskEnableUseWithInstancedSM))
				{
					Material->Modify();
					Material->bUsedWithInstancedStaticMeshes = true;
					Material->PostEditChange();
					FAGX_ObjectUtilities::SaveAsset(*Material);
				}
				else
				{
					// Clear the material selection.
					MatInterface = nullptr;
				}
			}
			else
			{
				// This is a material not part of the UE project itself. It may reside in the
				// installed Unreal Editor itself, or some plugin. We are not comfortable making
				// permanent changes to such materials, so we will prompt the user to do it
				// themselves.
				const FString Message =
					"The selected Material does not have Use With Instanced Static Meshes enabled, "
					"meaning that it cannot be used with the Track Mesh Renderer. You can enable "
					"this "
					"setting from the Material editor. The Material needs to be saved after these "
					"changes. It is recommended to make a copy and place the "
					"material within the project Contents, that way the behavior will be the same "
					"on any computer opening this project.";
				FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(Message);

				// Clear the material selection.
				MatInterface = nullptr;
			}
		}
	}
#endif
}

UAGX_TrackMeshRenderer::UAGX_TrackMeshRenderer()
{
	// Set this component to be ticked every frame so that it can synchronize
	// the visual track node instance transforms.
	PrimaryComponentTick.bCanEverTick = true;

	// \todo We want to synchronize visuals AFTER the physics have stepped. Find correct group!
	// PrimaryComponentTick.TickGroup = ??;

	// Set default values in inherited classes.
	// Make sure Unreal's default physics collision is disabled.
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UAGX_TrackMeshRenderer::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// \todo We do not need to sync visual transforms if physics simulation have not been
	//       stepped since last time we synchronized.
	SynchronizeVisuals();
}

TStructOnScope<FActorComponentInstanceData> UAGX_TrackMeshRenderer::GetComponentInstanceData() const
{
#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	UE_LOG(
		LogAGX, Verbose,
		TEXT("UAGX_TrackMeshRenderer::GetComponentInstanceData() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	return Super::GetComponentInstanceData();
}

void UAGX_TrackMeshRenderer::ApplyComponentInstanceData(
	struct FInstancedStaticMeshComponentInstanceData* ComponentInstanceData)
{
	Super::ApplyComponentInstanceData(ComponentInstanceData);

#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	UE_LOG(
		LogAGX, Verbose,
		TEXT("UAGX_TrackMeshRenderer::ApplyComponentInstanceData() for '%s' (UID: %i) in '%s'. "
			 "Phase "
			 "unknown."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	// \note Actually only want to execute the code below for second phase, after properties have
	//       been fully deserialized, but because we need to use the parent class' component
	//       instance data we have no way ourselves to pass the CacheApplyPhase to this function.
	// if (CacheApplyPhase == ECacheApplyPhase::PostUserConstructionScript)
	{
		const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
		if (bIsPlaying)
		{
			// In case the game is paused, update the render data here, so we can see visual changes
			// immediately even if there is no tick yet.
			SynchronizeVisuals();
		}
		else
		{
			// Rebind and synchronize after second phase of BP actor instance reconstruction, so
			// that the visualization includes the property changes that caused the reconstruction.
			RebindToTrackPreviewNeedsUpdateEvent();
		}
	}
}

void UAGX_TrackMeshRenderer::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR

void UAGX_TrackMeshRenderer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	UE_LOG(
		LogAGX, Verbose,
		TEXT("UAGX_TrackMeshRenderer::PostEditChangeProperty() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	const FName Property = GetFNameSafe(PropertyChangedEvent.Property);
	if (Property == GET_MEMBER_NAME_CHECKED(UAGX_TrackMeshRenderer, OverrideMaterials))
	{
		AGX_TrackMeshRenderer_helpers::EnsureValidRenderMaterial(*this);
	}

	// Update the render data both regardless of playing or not.
	// If not playing, we do no only update render data when something changes, so do it here.
	// If playing, we update it every tick, so not strictly necessary here, but in the scenario
	// were the user has paused the simulation and is fine tuning the visual scale, offset and
	// rotation, we want to react to those changing and update the render data directly here (since
	// the next tick doesn't happen until game is un-paused).
	SynchronizeVisuals();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UAGX_TrackMeshRenderer::PostLoad()
{
	Super::PostLoad();

	// Remove all instances in case they were saved during edit time.
	if (GetInstanceCount() > 0)
	{
		ClearInstances();
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// Bind to TrackPreviewNeedsUpdateEvent of the target track so that we know
		// when to update the track preview rendering while not playing.
		RebindToTrackPreviewNeedsUpdateEvent();
	}
}

void UAGX_TrackMeshRenderer::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	UE_LOG(
		LogAGX, Verbose,
		TEXT("UAGX_TrackMeshRenderer::OnAttachmentChanged() for '%s' (UID: %i) in '%s'."),
		*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()));
#endif

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// Target track might have changed, so rebind to the TrackPreviewNeedsUpdateEvent
		// of the potentially new target track.
		RebindToTrackPreviewNeedsUpdateEvent();
	}
}

void UAGX_TrackMeshRenderer::RebindToTrackPreviewNeedsUpdateEvent(bool bSynchronizeImmediately)
{
	if (IsBeingDestroyed())
	{
		return;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// Track preview only relevant when not playing.
		return;
	}

#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	UE_LOG(
		LogAGX, Verbose, TEXT("'%s' (UID: %i) in '%s' is rebinding TrackPreviewNeedsUpdateEvent."),
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
#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
		UE_LOG(
			LogAGX, Verbose,
			TEXT("'%s' (UID: %i) in '%s' is registering to TrackPreviewNeedsUpdateEvent "
				 "of '%s' (UID: %i) in '%s'"),
			*GetName(), GetUniqueID(), *GetNameSafe(GetOwner()), *Track->GetName(),
			Track->GetUniqueID(), *GetNameSafe(Track->GetOwner()));
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

UAGX_TrackComponent* UAGX_TrackMeshRenderer::FindTargetTrack()
{
	using namespace AGX_TrackMeshRenderer_helpers;
	return FindFirstParentComponentByClass<UAGX_TrackComponent>(this); // \todo Cache component!
}

void UAGX_TrackMeshRenderer::SetInstanceCount(int32 Count)
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

void UAGX_TrackMeshRenderer::SynchronizeVisuals()
{
	UAGX_TrackComponent* Track = FindTargetTrack();

#ifdef TRACK_MESH_RENDERER_DETAILED_LOGGING
	if (!IsValid(Track))
	{
		// \note This warning can happen during BP instance reconstruction when SynchronizeVisuals
		//       is called prematurely (i.e. before all properties have been fully initialized both
		//       on the renderer and the track component), but is no problem if it is called again
		//       later during the reconstruction when all properties have been fully deserialized.
		UE_LOG(
			LogAGX, Warning,
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
		UpdateInstanceTransform(
			i, NodeTransformsCache[i], /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ true);
	}
}

bool UAGX_TrackMeshRenderer::ComputeNodeTransforms(
	TArray<FTransform>& OutTransforms, UAGX_TrackComponent* Track)
{
	if (!IsValid(Track) || !Track->bEnabled || Track->Wheels.Num() == 0 ||
		Track->IsBeingDestroyed())
		return false;

	// Get node transforms either from the actual track when playing,
	// or from a generated preview if not playing.
	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
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
			ComputeVisualScaleAndOffset(VisualScale, VisualOffset, Track->GetNodeSize(0));
		}
		else
		{
			VisualScale = Scale;
			VisualOffset = Offset;
		}

		Track->GetNodeTransforms(OutTransforms, VisualScale, VisualOffset, Rotation.Quaternion());
	}
	else
	{
		// Get mesh instance transforms from preview data.
		FAGX_TrackPreviewData* Preview = Track->GetTrackPreview(
			/*bUpdateIfNecessary*/ true, /*bForceUpdate*/ false);

		if (!Preview || Preview->NodeTransforms.Num() <= 0)
		{
			return false;
		}
		check(Preview->NodeTransforms.Num() == Preview->NodeHalfExtents.Num());

		const FVector PhysicsNodeSize = 2 * Preview->NodeHalfExtents[0];
		const FVector BodyFrameToNodeCenter = FVector(0, 0, 0.5f * PhysicsNodeSize.Z);
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
			const FVector WorldOffset = Preview->NodeTransforms[i].GetRotation().RotateVector(
				VisualOffset + BodyFrameToNodeCenter);

			OutTransforms[i].SetScale3D(VisualScale);
			OutTransforms[i].SetRotation(
				Preview->NodeTransforms[i].GetRotation() * Rotation.Quaternion());
			OutTransforms[i].SetLocation(Preview->NodeTransforms[i].GetTranslation() + WorldOffset);
		}
	}
	return true;
}

bool UAGX_TrackMeshRenderer::ComputeVisualScaleAndOffset(
	FVector& OutVisualScale, FVector& OutVisualOffset, const FVector& PhysicsNodeSize) const
{
	const FVector LocalMeshBoundsSize = LocalMeshBoundsMax - LocalMeshBoundsMin;
	const FVector LocalBoundsCenter = LocalMeshBoundsMin + LocalMeshBoundsSize * 0.5f;

	if (FMath::IsNearlyZero(LocalMeshBoundsSize.X) || FMath::IsNearlyZero(LocalMeshBoundsSize.Y) ||
		FMath::IsNearlyZero(LocalMeshBoundsSize.Z))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to compute visual Scale and Offset for '%s' in '%s' because "
				 "LocalMeshBoundsMax is too close too LocalMeshBoundsMin. Render mesh will use "
				 "identity transformation."),
			*GetName(), *GetNameSafe(GetOwner()));
		OutVisualScale = FVector::OneVector;
		OutVisualOffset = FVector::ZeroVector;
		return false;
	}

	OutVisualScale = PhysicsNodeSize / LocalMeshBoundsSize;
	OutVisualOffset =
		-LocalBoundsCenter * Scale; // times scale to convert offset to post-scale coordinates
	return true;
}

#undef DEFAULT_VISUAL_SCALE
#undef DEFAULT_VISUAL_OFFSET
#undef LOCTEXT_NAMESPACE
