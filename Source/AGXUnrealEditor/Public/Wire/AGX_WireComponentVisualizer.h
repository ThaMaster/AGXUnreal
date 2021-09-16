#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

class UAGX_WireComponent;

/**
 * The Wire Component Visualizer uses lines and points to produce a simple visualization of a wire,
 * both its route nodes, while editing, and the simulation nodes, during a Play In Editor session.
 *
 * While in edit mode the Wire Component Visualizer provides functionality to duplicate, move, and
 * delete nodes.
 *
 * The Wire Component Visualizer maintains a node selection state which can be used to provide
 * additional manipulation operations on the node, such as from a Details Customization.
 */
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

	virtual bool IsVisualizingArchetype() const override;

	virtual void EndEditing() override;

	//~ End FComponentVisualizer Interface

	bool HasValidNodeSelection() const;
	bool HasValidWinchSelection() const;
	UAGX_WireComponent* GetSelectedWire() const;
	int32 GetSelectedNodeIndex() const;
	void SetSelectedNodeIndex(int32 InIndex);
	void ClearSelection();

private:
	void OnDeleteKey();
	bool CanDeleteKey() const;

private:
	/// The index of the currently selected node, if any. INDEX_NONE otherwise.
	int32 SelectedNodeIndex = INDEX_NONE;

	EWireSide SelectedWinch = EWireSide::None;
	EWinchSide SelectedWinchSide = EWinchSide::None;

	/**
	 * Property path from the owning Actor to the Wire Component of the currently selected wire. We
	 * must use a path instead of a UAGX_WireComponent* because during Blueprint Reconstruction
	 * the Wire Component will be replaced by a new instance.
	 */
	FComponentPropertyPath WirePropertyPath;

	FProperty* RouteNodesProperty = nullptr;
	FProperty* BeginWinchProperty = nullptr;
	FProperty* EndWinchProperty = nullptr;

	/// True while a node duplication move is in progress, so that we don't create a new each frame.
	bool bIsDuplicatingNode = false;

	TSharedPtr<FUICommandList> CommandList;

	/// A library of helper function manipulating the private state of FAGX_WireComponentVisualizer.
	friend class FWireVisualizerOperations;
};
