// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ShovelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "ActorEditorUtils.h"
#include "Terrain/AGX_ShovelComponent.h"

#define LOCTEXT_NAMESPACE "AGX_ShovelComponentVisualizer"
#define MEMBER(Name) GET_MEMBER_NAME_CHECKED(UAGX_ShovelComponent, Name)

FAGX_ShovelComponentVisualizer::FAGX_ShovelComponentVisualizer()
{
	UClass* Class = UAGX_ShovelComponent::StaticClass();
	TopEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(TopEdge));
	CuttingEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingEdge));
	CuttingDirectionProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingDirection));

	// Here is where we would register the commands class and create a command list, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

FAGX_ShovelComponentVisualizer::~FAGX_ShovelComponentVisualizer()
{
	// Here is where we would unregister the commands class, if we ever have the n eed for keyboard
	// shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();

	// Here is where we would register command list actions and bind callbacks, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FComponentVisualizer::DrawVisualization(Component, View, PDI);

	const UAGX_ShovelComponent* Shovel = Cast<UAGX_ShovelComponent>(Component);
	if (Component == nullptr)
	{
		ClearSelection();
		return;
	}

	// Draw the top edge.
	const FVector BeginLocation = Shovel->TopEdge.Start.GetWorldLocation();
	const FVector EndLocation = Shovel->TopEdge.End.GetWorldLocation();
	FLinearColor Color = FLinearColor::Red;
	PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground);
}

bool FAGX_ShovelComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	return FComponentVisualizer::VisProxyHandleClick(InViewportClient, VisProxy, Click);
}

bool FAGX_ShovelComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	return FComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);
}

bool FAGX_ShovelComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	return FComponentVisualizer::HandleInputDelta(
		ViewportClient, Viewport, DeltaTranslate, DeltaRotate, DeltaScale);
}

bool FAGX_ShovelComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return FComponentVisualizer::HandleInputKey(ViewportClient, Viewport, Key, Event);
}

bool FAGX_ShovelComponentVisualizer::IsVisualizingArchetype() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
	AActor* Owner = Shovel->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

void FAGX_ShovelComponentVisualizer::EndEditing()
{
	FComponentVisualizer::EndEditing();
}

bool FAGX_ShovelComponentVisualizer::HasValidFrameSection() const
{
	return GetSelectedFrame() != nullptr;
}

FAGX_Frame* FAGX_ShovelComponentVisualizer::GetSelectedFrame() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return nullptr;
	}
	switch (SelectedFrameSource)
	{
		case EAGX_ShovelFrame::None: return nullptr;
		case EAGX_ShovelFrame::CuttingDirection: return &Shovel->CuttingDirection;
		case EAGX_ShovelFrame::CuttingEdgeBegin: return &Shovel->CuttingEdge.Start;
		case EAGX_ShovelFrame::CuttingEdgeEnd: return &Shovel->CuttingEdge.End;
		case EAGX_ShovelFrame::TopEdgeBegin: return &Shovel->TopEdge.Start;
		case EAGX_ShovelFrame::TopEdgeEnd: return &Shovel->TopEdge.End;
	}
}

EAGX_ShovelFrame FAGX_ShovelComponentVisualizer::GetSelectedFrameSource() const
{
	return SelectedFrameSource;
}

void FAGX_ShovelComponentVisualizer::ClearSelection()
{
	SelectedFrameSource = EAGX_ShovelFrame::None;
	ShovelPropertyPath.Reset();
}

UAGX_ShovelComponent* FAGX_ShovelComponentVisualizer::GetSelectedShovel() const
{
	return Cast<UAGX_ShovelComponent>(ShovelPropertyPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
#undef MEMBER
