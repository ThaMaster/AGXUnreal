// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "Vehicle/AGX_TrackWheel.h"

// AGX Dynamics for Unreal barrier includes.
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TrackComponent.generated.h"

class UAGX_ShapeMaterialBase;
class UAGX_TrackWheelComponent;
class UAGX_TrackPropertiesBase;
class UAGX_TrackInternalMergePropertiesBase;

/**
 * Object holding track node transforms and sizes generated before the actual simulation
 * has started, in order to visualize a preview of the track.
 */
class AGXUNREAL_API FAGX_TrackPreviewData
{
public:
	TArray<FTransform> NodeTransforms;
	TArray<FVector> NodeHalfExtents;
};

/**
 * Experimental
 * This feature is experimental and may change in the next release. We do not
 * guarantee backwards compatibility.
 *
 *
 * Given a set of wheels, automatically generates a continuous track with a given number of shoes
 * (nodes).
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_TrackComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_TrackComponent();

	/**
	 * Whether the AGX Dynamics Tracks should be created or not. Cannot be changed while playing.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bEnabled;

	/**
	 * Number of nodes in the track.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	int NumberOfNodes;

	/**
	 * Width of the track nodes [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float Width;

	/**
	 * Thickness of the track nodes [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float Thickness;

	/**
	 * Value (distance) of how much shorter each node should be which causes tension
	 * in the system of tracks and wheels [cm].
	 *
	 * Since contacts and other factors are included it's not possible to know the exact
	 * tension after the system has been created.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float InitialDistanceTension;

	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_ShapeMaterialBase* ShapeMaterial;

	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_TrackPropertiesBase* TrackProperties;

	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_TrackInternalMergePropertiesBase* InternalMergeProperties;

	/**
	 * List of collision groups that the track nodes are part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	TArray<FName> CollisionGroups;

	/**
	 * The mass of the each track node Rigid Body [kg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track", Meta = (EditCondition = "!bAutoGenerateMass"))
	float NodeMass;

	/**
	 * Whether the track node mass should be computed automatically from the collision shape
	 * volume and material density.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGenerateMass;

	/**
	 * Center of mass offset [cm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track",
		Meta = (EditCondition = "!bAutoGenerateCenterOfMassOffset"))
	FVector NodeCenterOfMassOffset;

	/**
	 * Whether the center of mass offset should be computed automatically.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGenerateCenterOfMassOffset;

	/**
	 * The three-component diagonal of the inertia tensor [kgm^2].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track",
		Meta = (EditCondition = "!bAutoGeneratePrincipalInertia"))
	FVector NodePrincipalInertia;

	/**
	 * Whether the principal inertia should be computed automatically.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGeneratePrincipalInertia;

	/**
	 * An array of track wheels that are used to route the track and determine interaction
	 * characteristics between the wheel geometry and the track nodes.
	 *
	 * At BeginPlay these nodes are used to initialize the track and after that the wheel objects
	 * aren't used anymore.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Wheels")
	TArray<FAGX_TrackWheel> Wheels;

	/**
	 * Whether to show debug visualization when the track is selected in the Editor (must be ejected
	 * from Pawn if playing).
	 *
	 * -- Track Nodes --
	 *
	 *  Rigid Body Frame:     Red, green, blue XYZ Axes (based at body position)
	 *  Collision Box:        Black Wire Box
	 *  Center Of Mass:       Purple Point [Play Only]
	 *  Hinge Rotation Axis:  White Arrow (based at hinge position) [Play Only]
	 *
	 *
	 * -- Wheels --
	 *
	 *  Radius And Frame:      Green Cylinder
	 *  Rotation Axis:         Green Arrow
	 *
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bShowEditorDebugGraphics;

	/**
	 * Whether the debug graphics should colorize the collision boxes based on merged states
	 * (black means no merge).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bColorizeMergedBodies;

	/**
	 * Whether this component should try to update the Track Preview (debug rendering and actual
	 * track rendering) automatically whenever a property changes.
	 *
	 * For manual update, click the 'Update Preview' from the Track Component's Details Panel,
	 * or the 'Update Visuals' button from the Track Renderer's Detail Panel.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bAutoUpdateTrackPreview;

	/**
	 * Event that broadcasts whenever the outside-of-play Track Preview Data has changed.
	 * Typically useful for a track rendering component that outside of Play want to update its
	 * track preview rendering only when something has changed.
	 */
	DECLARE_EVENT_OneParam(FLayerViewModel, FTrackPreviewNeedsUpdateEvent, UAGX_TrackComponent*)
		FTrackPreviewNeedsUpdateEvent& GetTrackPreviewNeedsUpdateEvent()
	{
		return TrackPreviewNeedsUpdateEvent;
	}

	/**
	 * Get the number of nodes in this track.
	 *
	 * This is the same as the number of simulated Rigid Bodies during Play, and the number of
	 * preview nodes in the preview data during editing.
	 *
	 * During editing, i.e. not Play, this number is read from the Track Preview, if one is
	 * available. If no Track Preview is available then the NumberOfNodes property is returned.
	 * During Play, including Play In Editor, this will return the number of nodes created and
	 * simulated by AGX Dynamics.
	 *
	 * @return The number track nodes.
	 */
	int32 GetNumNodes() const;

	/**
	 * Get the transform of the center point of all track nodes.
	 *
	 * Scale is set to LocalScale and location is offset by LocalOffset * Rotation.
	 *
	 * During editing the node transform are taken from the preview data. If no prevew data is
	 * available then OutTransform is emptied. During Play, including Play-In-Editor, the node
	 * transforms are read from the AGX Dynamics Track instance. Used for track rendering while
	 * playing.
	 *
	 * @param OutTransform Array filled with one transform per node.
	 * @param LocalScale All transforms' Scale is set to this value.
	 * @param LocalOffset All transforms' Location is offset by this vector, rotated by each transforms' rotation.
	 */
	void GetNodeTransforms(
		TArray<FTransform>& OutTransforms, const FVector& LocalScale,
		const FVector& LocalOffset) const;

	/**
	 * Get the sizes of all track nodes.
	 *
	 * Only valid to call during simulation after an AGX Dynamics Track has been created.
	 *
	 * This is the full size, not the half-extent, of each node.
	 */
	void GetNodeSizes(TArray<FVector>& OutNodeSizes) const;

	/**
	 * Get the size of a track node.
	 *
	 * If Index is out of bounds then the zero vector is returned.
	 *
	 * Only valid to call during simulation after an AGX Dynamics Track has been created.
	 *
	 * @param Index The index of the node to to get.
	 * @return The size of the node, or the zero vector if index is out of bounds.
	 * @see GetNumNodes
	 */
	FVector GetNodeSize(int32 Index) const;

	/**
	 * Returns a preview of the track node transforms and sizes. Should only be used when not
	 * playing.
	 * @param bUpdateIfNecessary Update preview data if it has been flagged as dirty.
	 * @param bForceUpdate Update preview data regardless of dirty flag.
	 */
	FAGX_TrackPreviewData* GetTrackPreview(
		bool bUpdateIfNecessary = true, bool bForceUpdate = false) const;

public:
	/**
	 * Call whenever a property etc that affects the track preview data has changed.
	 * Will set bUpdateIfNecessary to true, and broadcast the TrackPreviewNeedsUpdateEvent
	 * to inform external classes that if GetTrackPreview() is called again it will
	 * generate a new track preview based on recent property changes.
	 */
	void RaiseTrackPreviewNeedsUpdate(bool bDoNotBroadcastIfAlreadyRaised = true);

	/// Get the native AGX Dynamics representation of this track. Create it if necessary.
	FTrackBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this track. May return nullptr.
	FTrackBarrier* GetNative();

	const FTrackBarrier* GetNative() const;

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostLoad() override;
	// ~End UObject interface.

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	void ApplyComponentInstanceData(
		const FActorComponentInstanceData* Data, ECacheApplyPhase CacheApplyPhase);
	//~ End UActorComponent Interface

	// ~Begin USceneComponent interface.
#if WITH_EDITOR
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
#endif

private:
#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every UPROPERTY in PostEditChangeProperty and PostEditChangeChainProperty.
	void InitPropertyDispatcher();
#endif

	// Find and set OwningActor of RigidBodyReferences and SceneComponentReferences which
	// in case this component is part of a Blueprint Actor.
	void ResolveComponentReferenceOwningActors();

	// Create the native AGX Dynamics object.
	void CreateNative();

	// Set ShapeMaterial assignment on native. Create native ShapeMaterial if not yet created.
	void WriteShapeMaterialToNative();

	// Set TrackProperties assignment on native. Create native TrackProperties if not yet created.
	void WriteTrackPropertiesToNative();

	// Write UAGX_TrackInternalMergeProperties properties to native.
	void WriteInternalMergePropertiesToNative();

	// Write mass, center of mass, inertia tensor, and auto-generation flags to native bodies.
	void WriteMassPropertiesToNative();

	/**
	 * Should be called whenever properties (excluding transform and shapes) need to be pushed
	 * onto the native in runtime. Writes all properties to native, except for those only
	 * used during initialization.
	 */
	void WritePropertiesToNative();

private:
	// The AGX Dynamics object only exists while simulating.
	// Initialized in BeginPlay and released in EndPlay.
	FTrackBarrier NativeBarrier;

	mutable TSharedPtr<FAGX_TrackPreviewData> TrackPreview = nullptr;
	mutable bool bTrackPreviewNeedsUpdate;

	FTrackPreviewNeedsUpdateEvent TrackPreviewNeedsUpdateEvent;
};

/**
 * This struct's only purpose is to inform UAGX_TrackComponent when a Blueprint Reconstruction is
 * complete, i.e. when properties have been deserialized and instance data applied.
 *
 * It inherits FAGX_NativeOwnerInstanceData because UAGX_TrackComponent is a native owner.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TrackComponentInstanceData : public FAGX_NativeOwnerInstanceData
{
	GENERATED_BODY()

	FAGX_TrackComponentInstanceData() = default;
	FAGX_TrackComponentInstanceData(
		const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
		TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster);

	virtual ~FAGX_TrackComponentInstanceData() override = default;

	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;
};
