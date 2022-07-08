// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

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

	// \todo How to handle lifetimes of these material proxys? Crashes if we let a smart pointer
	//       delete them from destructor.. Temporary proxys like this seems to usually be handled
	//       by using FMeshElementCollector.RegisterOneFrameMaterialProxy, but haven't found a way
	//       to access the collector from within the DrawVisualization() function.
	// See also C:\Program Files\Epic Games\UE_4.25\Engine\Source\Runtime\Renderer\Private\SceneVisibility.cpp line 4224 FColoredMaterialRenderProxy
	// See also C:\Program Files\Epic Games\UE_4.25\Engine\Source\Editor\UnrealEd\Private\UnrealWidget.cpp line 177 UMaterialInstanceDynamic::Create
	FMaterialRenderProxy* MassCenterMaterialProxy; // mass center material proxy
	FMaterialRenderProxy* CollisionBoxMaterialProxy; // common collision box material proxy
	TMap<FLinearColor, FMaterialRenderProxy*> CollisionBoxMaterialProxies; // per-node color based on merged body state

};
