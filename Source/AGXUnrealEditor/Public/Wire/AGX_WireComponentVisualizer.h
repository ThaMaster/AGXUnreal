#pragma once

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_WireComponent;

class AGXUNREALEDITOR_API FAGX_WireComponentVisualizer : public FComponentVisualizer
{
public:
	//~ Begin FComponentVisualizer Interface

	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;

	virtual bool VisProxyHandleClick(
		FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;

	virtual bool GetWidgetLocation(
		const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

	virtual bool HandleInputDelta(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
		FRotator& DeltaRotate, FVector& DeltaScale) override;

	virtual void EndEditing() override;

	//~ End FComponentVisualizer Interface

private:
	int32 SelectedNodeIndex = INDEX_NONE;
	UAGX_WireComponent* SelectedWire;
};
