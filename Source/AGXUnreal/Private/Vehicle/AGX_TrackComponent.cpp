// Copyright 2023, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "CoreGlobals.h"
#include "GameFramework/Actor.h"
#include "Math/Quat.h"

UAGX_TrackComponent::UAGX_TrackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsOnUpdateTransform = true;
}

FAGX_TrackPreviewData* UAGX_TrackComponent::GetTrackPreview(
	bool bUpdateIfNecessary, bool bForceUpdate) const
{
	// Avoid getting Track Preview if no valid license is available since this will spam license
	// errors in the log.
	if (!MayAttemptTrackPreview)
	{
		MayAttemptTrackPreview = FAGX_Environment::GetInstance().EnsureAGXDynamicsLicenseValid();
		if (!MayAttemptTrackPreview)
		{
			return nullptr;
		}
	}

	if (IsBeingDestroyed() || !bEnabled)
	{
		return nullptr;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		return nullptr; // no preview while playing
	}

	if (!TrackPreview.IsValid() || bTrackPreviewNeedsUpdate || bForceUpdate)
	{
		// Generate track preview data, without actually creating a real track or bodies.

		if (!TrackPreview.IsValid())
		{
			TrackPreview = MakeShared<FAGX_TrackPreviewData>();
		}

		// Create AGX barrier wheel descs.
		TArray<FTrackBarrier::FTrackWheelDescription> WheelDescs;
		WheelDescs.Reserve(Wheels.Num());
		for (const auto& Wheel : Wheels)
		{
			UAGX_RigidBodyComponent* Body = Wheel.RigidBody.GetRigidBody();
			if (!Body)
				continue;

			// Make sure the world transform is up-to-date.
			Body->ConditionalUpdateComponentToWorld();

			// Create wheel data.
			FTrackBarrier::FTrackWheelDescription Desc;
			Desc.Model = static_cast<decltype(Desc.Model)>(Wheel.Model);
			Desc.Radius = Wheel.Radius;
			Desc.RigidBodyTransform = Body->GetComponentTransform();
			Wheel.GetTransformRelativeToBody(Desc.RelativePosition, Desc.RelativeRotation);
			WheelDescs.Add(Desc);
		}

		// Let AGX generate track nodes preview data.
		FTrackBarrier::GetPreviewData(
			TrackPreview->NodeTransforms, TrackPreview->NodeHalfExtents, NumberOfNodes, Width,
			Thickness, InitialDistanceTension, WheelDescs);

		bTrackPreviewNeedsUpdate = false;
	}

	check(TrackPreview.IsValid());
	return TrackPreview.Get();
}

void UAGX_TrackComponent::RaiseTrackPreviewNeedsUpdate(bool bDoNotBroadcastIfAlreadyRaised)
{
	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		return; // Track preview is only relevant when not playing.
	}

	bool bShouldBroadcastEvent = !bTrackPreviewNeedsUpdate || !bDoNotBroadcastIfAlreadyRaised;

	bTrackPreviewNeedsUpdate = true;

	if (bShouldBroadcastEvent)
	{
		TrackPreviewNeedsUpdateEvent.Broadcast(this);
	}
}

void UAGX_TrackComponent::CopyFrom(const FTrackBarrier& Barrier)
{
	NumberOfNodes = Barrier.GetNumNodes();
	Width = static_cast<float>(Barrier.GetWidth());
	Thickness = static_cast<float>(Barrier.GetThickness());
	InitialDistanceTension = static_cast<float>(Barrier.GetInitialDistanceTension());
	CollisionGroups = Barrier.GetCollisionGroups();
	ImportGuid = Barrier.GetGuid();

	if (Barrier.GetNumNodes() > 0)
	{
		FRigidBodyBarrier FirstBodyBarrier = Barrier.GetNodeBody(0);
		if (FirstBodyBarrier.HasNative())
		{
			const FMassPropertiesBarrier& MassProperties = FirstBodyBarrier.GetMassProperties();

			bAutoGenerateMass = MassProperties.GetAutoGenerateMass();
			bAutoGenerateCenterOfMassOffset = MassProperties.GetAutoGenerateCenterOfMassOffset();
			bAutoGeneratePrincipalInertia = MassProperties.GetAutoGeneratePrincipalInertia();
			NodeMass = MassProperties.GetMass();
			NodeCenterOfMassOffset = FirstBodyBarrier.GetCenterOfMassOffset();
			NodePrincipalInertia = MassProperties.GetPrincipalInertia();
		}
	}
}

int32 UAGX_TrackComponent::GetNumNodes() const
{
	if (HasNative())
	{
		return GetNative()->GetNumNodes();
	}
	else if (TrackPreview.IsValid())
	{
		return TrackPreview->NodeTransforms.Num();
	}
	else
	{
		return NumberOfNodes;
	}
}

void UAGX_TrackComponent::GetNodeTransforms(
	TArray<FTransform>& OutTransforms, const FVector& LocalScale, const FVector& LocalOffset,
	const FQuat& LocalRotation) const
{
	if (HasNative())
	{
		GetNative()->GetNodeTransforms(OutTransforms, LocalScale, LocalOffset, LocalRotation);
	}
	else if (TrackPreview.IsValid())
	{
		const int32 NumNodes = TrackPreview->NodeTransforms.Num();
		// Retain the container buffer so that the same transform cache can be reused for multiple
		// tracks without reallocation every time.
		OutTransforms.SetNum(NumNodes, /*bAllowShrinking*/ false);
		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FTransform& Source = TrackPreview->NodeTransforms[I];
			FTransform& Target = OutTransforms[I];
			Target.SetScale3D(LocalScale);
			Target.SetRotation(Source.GetRotation() * LocalRotation);
			const FVector WorldOffset = Source.GetRotation().RotateVector(LocalOffset);
			Target.SetLocation(Source.GetLocation() + WorldOffset);
		}
	}
	else
	{
		// Retain the container buffer so that the same transform cache can be reused for multiple
		// tracks without reallocation every time.
		OutTransforms.SetNum(0, /*bAllowShrinking*/ false);
	}
}

void UAGX_TrackComponent::GetNodeSizes(TArray<FVector>& OutNodeSizes) const
{
	if (!HasNative())
	{
		return;
	}

	GetNative()->GetNodeSizes(OutNodeSizes);
}

FVector UAGX_TrackComponent::GetNodeSize(int32 Index) const
{
	if (!HasNative())
	{
		return FVector::ZeroVector;
	}

	return GetNative()->GetNodeSize(Index);
}

FTrackBarrier* UAGX_TrackComponent::GetOrCreateNative()
{
	if (!HasNative() && bEnabled)
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Track '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	return GetNative();
}

FTrackBarrier* UAGX_TrackComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FTrackBarrier* UAGX_TrackComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

bool UAGX_TrackComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_TrackComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_TrackComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_TrackComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after InitProperties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	//
	// The intention is to provide by default a local scope that is the Actor outer that this
	// Component is part of. If the OwningActor is set anywhere else, such as in the Details Panel,
	// then that "else" should overwrite the value set here shortly.
	//
	// We use GetTypedOuter because we worry that in some cases the Owner may not yet have been set
	// but there will always be an outer chain. This worry may be unfounded.
	ResolveComponentReferenceOwningActors();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

bool UAGX_TrackComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, bEnabled),
			GET_MEMBER_NAME_CHECKED(ThisClass, NumberOfNodes),
			GET_MEMBER_NAME_CHECKED(ThisClass, Width),
			GET_MEMBER_NAME_CHECKED(ThisClass, Thickness),
			GET_MEMBER_NAME_CHECKED(ThisClass, InitialDistanceTension)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}

void UAGX_TrackComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TrackComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// \note We trigger dispatch both here and from PostEditChangeChainProperty, because
	// for example when editing a mass property while playing on a BP Actor Instance the
	// PostEditChangeChainProperty appears to be called too late in the reconstruction.
	//
	// \todo Check if this problem was "fixed" with merge request !583 - Call the Property Changed
	// callback on all selected objects.
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent);

	// Track Preview needs update if any of NumberOfNodes, Width, Thickness, or
	// InitialDistanceTension has changed.
	if (bAutoUpdateTrackPreview)
	{
		RaiseTrackPreviewNeedsUpdate();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UAGX_TrackComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	ResolveComponentReferenceOwningActors();
}

void UAGX_TrackComponent::PostLoad()
{
	Super::PostLoad();

	// It seems that because the wheels array, that the component references lives in, are sometimes
	// not yet populated in PostInitProperties(), which means we cannot resolve the owning actors at
	// that time. Therefore, we try to resolve the owning actors from here too, when the wheels
	// should be populated and ready.
	ResolveComponentReferenceOwningActors();

	RaiseTrackPreviewNeedsUpdate();
}

void UAGX_TrackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative() && bEnabled && !GIsReconstructingBlueprintInstances)
	{
		// Do not create a native AGX Dynamics object if GIsReconstructingBlueprintInstances is set.
		// That means that we're being created as part of a Blueprint Reconstruction and we will
		// soon be assigned the native that the reconstructed Track Component had, if any, in
		// ApplyComponentInstanceData.
		CreateNative();
		check(HasNative()); /// @todo Consider better error handling than check.
	}
}

void UAGX_TrackComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_TrackComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Track.
		//
		// But what if the Track isn't currently part of any Simulation? Can we guarantee that
		// something will keep the Track instance alive? Should we do explicit incref/decref
		// on the Track in GetNativeAddress / SetNativeAddress?
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		// This object is being destroyed / removed from a Play session that will continue without
		// it, so there will be no global cleanup of everything, so we must cleanup after ourself.
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			if (!Sim->HasNative())
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Track '%s' in '%s' tried to get Simulation, but returned simulation has "
						 "no native."),
					*GetName(), *GetNameSafe(GetOwner()));
				return;
			}

			// \todo Want to use AAGX_Simulation::Remove, but there's no overload taking a
			// TrackComponent
			//       (or LinkedStructure or Assembly), so we use a work-around in the TrackBarrier.
			const bool Result = GetNative()->RemoveFromSimulation(*Sim->GetNative());
		}
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_TrackComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_TrackComponentInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_TrackComponent::ApplyComponentInstanceData(
	const FActorComponentInstanceData* Data, ECacheApplyPhase CacheApplyPhase)
{
	if (CacheApplyPhase == ECacheApplyPhase::PostUserConstructionScript)
	{
		// In the case of BP Actor Instances, there can be wheels added to the instance
		// in the level in addition to the wheels on the CDO. During BP instance reconstruction,
		// those additional wheels are not added until instance has been fully deserialized.
		// Therefore, we here make sure that the resolving of OwningActor in RigidBodyRefernce
		// and SceneComponentReference is done for those additional wheels.
		ResolveComponentReferenceOwningActors();

		// Call this to re-register this reconstructed track component to
		// UAGX_TrackInternalMergeProperties.
		WriteInternalMergePropertiesToNative();

		// Mark the Track Preview Data for update after all BP instance data has been deserialized.
		if (bAutoUpdateTrackPreview)
		{
			RaiseTrackPreviewNeedsUpdate();
		}
	}
}

#if WITH_EDITOR

void UAGX_TrackComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	// \todo This event does not seem to be called when drag-moving an actor/component,
	//       but not when writing values directly in the Detail Panel transform input fields.

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// \note Actually moving the TransformComponent does not itself means that track preview
		//       needs update, but it's likely that this happened because the owning actor was
		//       moved, which usually means that all wheels moved.
		if (bAutoUpdateTrackPreview)
		{
			constexpr bool bDoNotBroadcastIfAlreadyRaised = true;
			RaiseTrackPreviewNeedsUpdate(bDoNotBroadcastIfAlreadyRaised);
		}
	}
}

void UAGX_TrackComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Assets.

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, ShapeMaterial),
		[](ThisClass* Self) { Self->UpdateNativeMaterial(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, TrackProperties),
		[](ThisClass* Self) { Self->WriteTrackPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, InternalMergeProperties),
		[](ThisClass* Self) { Self->WriteInternalMergePropertiesToNative(); });

	// Mass Properties.

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodeMass),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGenerateMass),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodeCenterOfMassOffset),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGenerateCenterOfMassOffset),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodePrincipalInertia),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGeneratePrincipalInertia),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });
}

#endif

void UAGX_TrackComponent::ResolveComponentReferenceOwningActors()
{
	// Make Track Wheels search for Rigid Body Components and Frame Defining Components
	// in the same Actor as this Track Component is in, unless another owner has already
	// been specified. This resolving of owning actors is typically necessary for Blueprint Actors
	// because which their OwningActor properties needs to be null during editing of the default
	// actor, and cannot be resolved to an the actual actor until the Blueprint Actor is actually
	// added to a level.
	for (FAGX_TrackWheel& Wheel : Wheels)
	{
		if (Wheel.RigidBody.OwningActor == nullptr)
		{
			Wheel.RigidBody.OwningActor = GetTypedOuter<AActor>();
		}
		if (Wheel.FrameDefiningComponent.OwningActor == nullptr)
		{
			Wheel.FrameDefiningComponent.OwningActor = GetTypedOuter<AActor>();
		}
	}
}

void UAGX_TrackComponent::CreateNative()
{
	// Check preconditions.
	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on Track Component '%s' in '%s' even though a Native already "
				 "exists. Doing nothing"),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}
	if (!bEnabled)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on disabled Track Component '%s' in '%s'. Doing nothing."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}
	if (GIsReconstructingBlueprintInstances)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on a Track Component while a Blueprint Reconstruction is in "
				 "progress. This is not allowed because during Blueprint Reconstruction AGX "
				 "Dynamic objects are stashed in an Actor Component Instance Data and will be "
				 "reused in the new AGX Actor Components. The Track Component is '%s' in '%s'. "
				 "Doing nothing."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	NativeBarrier.AllocateNative(NumberOfNodes, Width, Thickness, InitialDistanceTension);
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Track Component '%s' in '%s' could not allocate native AGX Dynamics instance."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this);
	if (!IsValid(Sim) || !Sim->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Track '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned a nullptr or a simulation without a native."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	ResolveComponentReferenceOwningActors();
	for (FAGX_TrackWheel& Wheel : Wheels)
	{
		// Validate and get the Rigid Body Component.
		UAGX_RigidBodyComponent* Body = Wheel.RigidBody.GetRigidBody();
		if (!IsValid(Body) || Body->GetOrCreateNative() == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Track initialization for '%s' in '%s'' encountered a track wheel with a "
					 "Rigid Body Component or native that is null or invalid. The wheel will be "
					 "ignored."),
				*GetName(), *GetNameSafe(GetOwner()));
			continue;
		}

		// Compute wheel position and rotation relative to the body.
		FVector RelPos;
		FQuat RelRot;
		bool transformOK = Wheel.GetTransformRelativeToBody(RelPos, RelRot);
		check(transformOK);

		// Add native wheel to native Track.
		NativeBarrier.AddTrackWheel(
			static_cast<uint8>(Wheel.Model), Wheel.Radius, *Body->GetOrCreateNative(), RelPos,
			RelRot, Wheel.bSplitSegments, Wheel.bMoveNodesToRotationPlane, Wheel.bMoveNodesToWheel);
	}

	// Set TrackProperties BEFORE adding track to simulation (i.e. triggering track initialization),
	// because some properties in TrackProperties affects the track initialization algorithm.
	WriteTrackPropertiesToNative();

	// \todo Want to use AAGX_Simulation::Add, but there's no overload taking a Track Component
	//       (or LinkedStructure or Assembly), so we use a work-around in the TrackBarrier.
	const bool Result = GetNative()->AddToSimulation(*Sim->GetNative());
	if (!Result)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to add '%s' in '%s' to Simulation. Add() returned false. "
				 "The Log category AGXDynamicsLog may contain more information about the failure."),
			*GetName(), *GetNameSafe(GetOwner()));
	}

	UpdateNativeProperties();
}

void UAGX_TrackComponent::UpdateNativeMaterial()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (ShapeMaterial == nullptr)
	{
		GetNative()->ClearMaterial();
		return;
	}

	// Create instance if necessary.
	UAGX_ShapeMaterial* MaterialInstance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(GetWorld()));
	check(MaterialInstance);
	// Replace asset reference with instance reference.
	if (ShapeMaterial != MaterialInstance)
	{
		ShapeMaterial = MaterialInstance;
	}

	const FShapeMaterialBarrier* MaterialBarrier =
		MaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
	check(MaterialBarrier);

	// Assign native.
	GetNative()->SetMaterial(*MaterialBarrier);
}

void UAGX_TrackComponent::WriteTrackPropertiesToNative()
{
	if (!HasNative() || GetWorld() == nullptr || !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (TrackProperties)
	{
		// Create instance if necessary.
		UAGX_TrackProperties* TrackPropertiesInstance =
			TrackProperties->GetOrCreateInstance(GetWorld());
		check(TrackPropertiesInstance);

		// Replace asset reference with instance reference.
		if (TrackPropertiesInstance != TrackProperties)
		{
			TrackProperties = TrackPropertiesInstance;
		}

		// Assign native.
		const FTrackPropertiesBarrier* TrackPropertiesBarrier =
			TrackPropertiesInstance->GetOrCreateNative();
		check(TrackPropertiesBarrier);
		GetNative()->SetProperties(*TrackPropertiesBarrier);
	}
	else
	{
		GetNative()->ClearProperties();
	}
}

void UAGX_TrackComponent::WriteInternalMergePropertiesToNative()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	const UWorld& World = *GetWorld();

	// \todo If InternalMergeProperties was switched out or set to None during Play, unregister this
	//       track from the previously set InternalMergePropertiesInstance. It is not strictly
	//       required though because UAGX_TrackInternalMergeProperties checks if registered target
	//       tracks point back to itself before applying any changes to the native.

	//// \todo Do this in a more performance friendly way. For example we could keep a reference
	////       to the previous InternalMergeProperties, or unregister from within PreEditChange()?
	// for (TObjectIterator<UAGX_TrackInternalMergeProperties> It; It; ++It)
	//{
	//	if (It->GetWorld() == GetWorld())
	//	{
	//		It->UnregisterTargetTrack(this);
	//	}
	//}

	if (InternalMergeProperties)
	{
		// Create instance if necessary.
		UAGX_TrackInternalMergeProperties* InternalMergePropertiesInstance =
			static_cast<UAGX_TrackInternalMergeProperties*>(
				InternalMergeProperties->GetOrCreateInstance(World));
		check(InternalMergePropertiesInstance);

		// Replace asset reference with instance reference.
		if (InternalMergePropertiesInstance != InternalMergeProperties)
		{
			InternalMergeProperties = InternalMergePropertiesInstance;
		}

		// Register this track as one of the target tracks.
		InternalMergePropertiesInstance->RegisterTargetTrack(this);
	}
	else
	{
		// \todo Want to call TrackInternalMergeProperties::resetToDefault(), but doing so gives
		//       very strange dynamics behaviour. It seems merge is supposed to become disbled but
		//       it still remains enabled somehow.
		//       Because of this, instead of reset we just set disabled here, which from a
		//       user perspective should lead to desired behaviour.
		GetNative()->InternalMergeProperties_SetEnableMerge(false);
	}
}

void UAGX_TrackComponent::WriteMassPropertiesToNative()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	const int NumNodes = NativeBarrier.GetNumNodes();
	for (int i = 0; i < NumNodes; ++i)
	{
		FRigidBodyBarrier BodyBarrier = NativeBarrier.GetNodeBody(i);
		check(BodyBarrier.HasNative());

		FMassPropertiesBarrier& MassProperties = BodyBarrier.GetMassProperties();

		MassProperties.SetAutoGenerateMass(bAutoGenerateMass);
		MassProperties.SetAutoGenerateCenterOfMassOffset(bAutoGenerateCenterOfMassOffset);
		MassProperties.SetAutoGeneratePrincipalInertia(bAutoGeneratePrincipalInertia);

		if (!bAutoGenerateMass)
		{
			MassProperties.SetMass(NodeMass);
		}
		if (!bAutoGenerateCenterOfMassOffset)
		{
			BodyBarrier.SetCenterOfMassOffset(NodeCenterOfMassOffset);
		}
		if (!bAutoGeneratePrincipalInertia)
		{
			MassProperties.SetPrincipalInertia(NodePrincipalInertia);
		}

		// Make sure mass properties are up-to-date with respect to the auto-generate
		// options set above. This is important because merely setting auto-generate flags
		// does not trigger an update of mass properties.
		BodyBarrier.UpdateMassProperties();
	}

	// Update UI with auto-generated values.
	if (NumNodes > 0)
	{
		FRigidBodyBarrier FirstBodyBarrier = NativeBarrier.GetNodeBody(0);
		check(FirstBodyBarrier.HasNative());

		const FMassPropertiesBarrier& MassProperties = FirstBodyBarrier.GetMassProperties();

		if (bAutoGenerateMass)
		{
			NodeMass = MassProperties.GetMass();
		}
		if (bAutoGenerateCenterOfMassOffset)
		{
			NodeCenterOfMassOffset = FirstBodyBarrier.GetCenterOfMassOffset();
		}
		if (bAutoGeneratePrincipalInertia)
		{
			NodePrincipalInertia = MassProperties.GetPrincipalInertia();
		}
	}
}

void UAGX_TrackComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		return;
	}

	NativeBarrier.SetName(GetName());

	// Set shape material on all native geometries.
	UpdateNativeMaterial();

	// Set track properties.
	WriteTrackPropertiesToNative();

	// Set track internal merge properties.
	WriteInternalMergePropertiesToNative();

	// Set collision groups on native.
	for (const FName& Group : CollisionGroups)
	{
		NativeBarrier.AddCollisionGroup(Group);
	}

	//  Set mass, center of mass, inertia tensor, and auto-gen properties on native rigid bodies.
	WriteMassPropertiesToNative();
}

FAGX_TrackComponentInstanceData::FAGX_TrackComponentInstanceData(
	const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
	TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster)
	: FAGX_NativeOwnerInstanceData(NativeOwner, SourceComponent, InDowncaster)
{
}

void FAGX_TrackComponentInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	Super::ApplyToComponent(Component, CacheApplyPhase);

	CastChecked<UAGX_TrackComponent>(Component)->ApplyComponentInstanceData(this, CacheApplyPhase);
}
