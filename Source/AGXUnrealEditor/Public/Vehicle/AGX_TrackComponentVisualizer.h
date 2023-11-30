// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

class FMaterialRenderProxy;

/**
 * The Track Component Visualizer provides debug visualization of collision boxes and center of mass positions of
 * the nodes of a Track Component, as well as the hinge joints connecting the nodes.
 */
class AGXUNREALEDITOR_API FAGX_TrackComponentVisualizer : public FComponentVisualizer
{
public:
	FAGX_TrackComponentVisualizer();
	~FAGX_TrackComponentVisualizer();

	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

private:

	TArray<FTrackBarrier::FVectorAndRotator> BodyTransformsCache;
	TArray<FTrackBarrier::FVectorAndRotator> HingeTransformsCache;
	TArray<FVector> MassCentersCache;
	TArray<FTrackBarrier::FVectorRotatorRadii> CollisionBoxesCache;
	TArray<FLinearColor> BodyColorsCache;
	TArray<FTrackBarrier::FVectorQuatRadius> WheelTransformsCache;
	TArray<FLinearColor> WheelColorsCache;

	FMaterialRenderProxy* MassCenterMaterialProxy; // mass center material proxy
	FMaterialRenderProxy* CollisionBoxMaterialProxy; // common collision box material proxy
	TMap<FLinearColor, FMaterialRenderProxy*> CollisionBoxMaterialProxies; // per-node color based on merged body state

};
