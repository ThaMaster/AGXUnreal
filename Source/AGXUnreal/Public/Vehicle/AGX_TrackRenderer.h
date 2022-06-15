// Author: VMC Motion Technologies Co., Ltd.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_SceneComponentReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include "AGX_TrackRenderer.generated.h"

class UAGX_TrackComponent;
class FTrackBarrier;


/**
 * Renderers all track nodes as instanced meshed using the same Static Mesh source.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TrackRenderer : public UHierarchicalInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UAGX_TrackRenderer();

	/**
	 * Local Scale to apply to the Static Mesh before synchronizing its position and rotation
	 * with a track node. Scale is relative to the original mesh size, not to the track node.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMesh,
		Meta = (EditCondition = "!bAutoScaleAndOffset"))
	FVector Scale;

	/**
	 * Local Translation to apply to the Static Mesh before synchronizing its position and
	 * rotation with a track node. Applied after Scale.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMesh,
		Meta = (EditCondition = "!bAutoScaleAndOffset"))
	FVector Offset;

	/** Whether to automatically compute the Scale and Offset necessary to fit the Static Mesh's
	 * local bounds (defined by Local Mesh Bounds Min/Max) to the physical track node box.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMesh)
	bool bAutoScaleAndOffset;

	/**
	 * The max-point of the axis-aligned local box volume which should be fitted to the
	 * physical track node box when  auto-computing mesh scale and offset.
	 *
	 * Set this to the maximum local coordinate that the physical track node should cover.
	 * If it is desired that visual parts (e.g. teeth or the part overlapping the neighboring shoe)
	 * appear outside of the physical track node, then set this value to not cover those parts,
	 * i.e. a bit smaller than the local bounding box (spanning all vertices).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMesh, Meta = (EditCondition = "bAutoScaleAndOffset"))
	FVector LocalMeshBoundsMax;

	/**
	 * The min-point of the axis-aligned local box volume which should be fitted to the
	 * physical track node box when auto-computing mesh scale and offset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMesh, Meta = (EditCondition = "bAutoScaleAndOffset"))
	FVector LocalMeshBoundsMin;

	//~ Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	virtual void ApplyComponentInstanceData(
		struct FInstancedStaticMeshComponentInstanceData* ComponentInstanceData) override;

	// ~Begin USceneComponent interface.
	virtual void OnAttachmentChanged() override;
	// ~End USceneComponent interface.

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	// ~End UObject interface.

	void RebindToTrackPreviewNeedsUpdateEvent(bool bSynchronizeImmediately = true);

	UAGX_TrackComponent* FindTargetTrack();

	void SynchronizeVisuals();

private:

	TArray<FTransform> NodeTransformsCache;

	void SetInstanceCount(int32 Count);

	bool ComputeNodeTransforms(TArray<FTransform>& OutTransforms, UAGX_TrackComponent* Track);

	bool ComputeVisualScaleAndOffset(
		FVector& OutVisualScale, FVector& OutVisualOffset, const FVector& PhysicsNodeSize) const;

};
