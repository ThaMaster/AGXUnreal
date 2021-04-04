#pragma once

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class AGXUNREALEDITOR_API FAGX_WireComponentVisualizer : public FComponentVisualizer
{
	//~ Begin FComponentVisualizer Interface

	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;

	virtual bool VisProxyHandleClick(
		FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;

	//~ End FComponentVisualizer Interface
};
