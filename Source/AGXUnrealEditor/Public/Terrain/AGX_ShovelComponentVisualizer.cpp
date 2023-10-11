// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ShovelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "ActorEditorUtils.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelHitProxies.h"
#include "Terrain/AGX_ShovelUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_ShovelComponentVisualizer"
#define MEMBER(Name) GET_MEMBER_NAME_CHECKED(UAGX_ShovelComponent, Name)

struct FShovelVisualizerOperations
{
	static bool ShovelProxyClicked(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		HShovelHitProxy& Proxy)
	{
		if (Shovel.HasNative())
		{
			// Not allowed to modify the shovel configuration once it has been instantiated.
			Visualizer.ClearSelection();
			return false;
		}

		if (Proxy.Frame == Visualizer.SelectedFrame)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearSelection();
			return true;
		}

		SelectFrame(Visualizer, Shovel, Proxy.Frame);
		return true;
	}

	static void SelectFrame(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		EAGX_ShovelFrame Frame)
	{
		Visualizer.SelectedFrame = Frame;
		Visualizer.ShovelPropertyPath = FComponentPropertyPath(&Shovel);
	}
};

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
	{
		const FVector BeginLocation = Shovel->TopEdge.Start.GetWorldLocation();
		const FVector EndLocation = Shovel->TopEdge.End.GetWorldLocation();
		FLinearColor Color = FLinearColor::White;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);

		PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeBegin));
		PDI->DrawPoint(BeginLocation, Color, FAGX_ShovelUtilities::HitProxySize, SDPG_Foreground);
		PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeEnd));
		PDI->DrawPoint(EndLocation, Color, FAGX_ShovelUtilities::HitProxySize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
	}

	// Draw the cutting edge.
	{
		const FVector BeginLocation = Shovel->CuttingEdge.Start.GetWorldLocation();
		const FVector EndLocation = Shovel->CuttingEdge.End.GetWorldLocation();
		FLinearColor Color = FLinearColor::Red;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);
	}

	// Draw the cutting direction.
	{
		const FVector BeginLocation = Shovel->CuttingDirection.GetWorldLocation();
		const FRotator Rotation = Shovel->CuttingDirection.GetWorldRotation();
		const FVector Direction = Rotation.RotateVector(FVector::ForwardVector);
		const FVector EndLocation = BeginLocation + 100 * Direction;
		const FLinearColor Color = FLinearColor::Red; //.Desaturate(0.5f);
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);
	}
}

bool FAGX_ShovelComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_ShovelComponent* Shovel = Cast<const UAGX_ShovelComponent>(VisProxy->Component);
	if (Shovel == nullptr)
	{
		// Clicked something not a shovel, deselect whatever we had selected before.
		ClearSelection();
		return false;
	}

	AActor* OldOwningActor = ShovelPropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Shovel->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		// Don't reuse selection data between Actors, it's completely different shovels.
		ClearSelection();
	}

	if (HShovelHitProxy* Proxy = HitProxyCast<HShovelHitProxy>(VisProxy))
	{
		return FShovelVisualizerOperations::ShovelProxyClicked(*this, *Shovel, *Proxy);
	}

	// Add additional proxy types here when needed.

	// The clicked proxy isn't a Shovel proxy, return false to pass on to the next handler in line.
	return false;
}


// Call by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected frame, if there is one.
bool FAGX_ShovelComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}

	if (FAGX_Frame* Frame = GetSelectedFrame())
	{
		OutLocation = Frame->GetWorldLocation();
		return true;
	}

	return false;
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
	switch (SelectedFrame)
	{
		case EAGX_ShovelFrame::None:
			return nullptr;
		case EAGX_ShovelFrame::CuttingDirection:
			return &Shovel->CuttingDirection;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
			return &Shovel->CuttingEdge.Start;
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return &Shovel->CuttingEdge.End;
		case EAGX_ShovelFrame::TopEdgeBegin:
			return &Shovel->TopEdge.Start;
		case EAGX_ShovelFrame::TopEdgeEnd:
			return &Shovel->TopEdge.End;
	}
}

EAGX_ShovelFrame FAGX_ShovelComponentVisualizer::GetSelectedFrameSource() const
{
	return SelectedFrame;
}

void FAGX_ShovelComponentVisualizer::ClearSelection()
{
	SelectedFrame = EAGX_ShovelFrame::None;
	ShovelPropertyPath.Reset();
}

UAGX_ShovelComponent* FAGX_ShovelComponentVisualizer::GetSelectedShovel() const
{
	return Cast<UAGX_ShovelComponent>(ShovelPropertyPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
#undef MEMBER
