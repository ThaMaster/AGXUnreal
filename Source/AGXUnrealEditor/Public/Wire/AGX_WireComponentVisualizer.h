#pragma once

// Unreal Engine includes.
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

class UAGX_WireComponent;

class AGXUNREALEDITOR_API FAGX_WireComponentVisualizer : public FComponentVisualizer
{
public:
	FAGX_WireComponentVisualizer();
	~FAGX_WireComponentVisualizer();

	//~ Begin FComponentVisualizer Interface

	virtual void OnRegister() override;

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

	virtual bool HandleInputKey(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
		EInputEvent Event) override;

	virtual void EndEditing() override;

	//~ End FComponentVisualizer Interface

	UAGX_WireComponent* GetSelectedWire();
	int32 GetSelectedNodeIndex();

private:
	void OnDeleteKey();
	bool CanDeleteKey() const;

private:
	int32 SelectedNodeIndex = INDEX_NONE;
	UAGX_WireComponent* SelectedWire = nullptr;

	bool bIsDuplicatingNode = false;

	TSharedPtr<FUICommandList> CommandList;
};
