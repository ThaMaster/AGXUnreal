#include "Wire/AGX_WireWinchVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/AGX_WireHitProxies.h"

// Unreal Engine includes.
#include "SceneManagement.h"

FAGX_WireWinchVisualizer::FAGX_WireWinchVisualizer()
{
	// Create and register command list here, if we ever need one.
}

FAGX_WireWinchVisualizer::~FAGX_WireWinchVisualizer()
{
	// Unregister command list here, if we ever need one.
}

void FAGX_WireWinchVisualizer::OnRegister()
{
	// Map command list here, if we ever need one.
}

void FAGX_WireWinchVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_WireWinchComponent* WinchComponent = Cast<UAGX_WireWinchComponent>(Component);
	if (WinchComponent == nullptr)
	{
		return;
	}

	AGX_WireVisualization_helpers::DrawWinch(WinchComponent, PDI);
}

bool FAGX_WireWinchVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_WireWinchComponent* Winch = Cast<const UAGX_WireWinchComponent>(VisProxy->Component);
	if (Winch == nullptr)
	{
		ClearSelection();
		return false;
	}

	AActor* OldOwningActor = WinchPropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Winch->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		ClearSelection();
	}

	if (HWinchLocationProxy* LocationProxy = HitProxyCast<HWinchLocationProxy>(VisProxy))
	{
		if (SelectedWinchSide == EWinchSide::Location)
		{
			// Clicking a selected handle deselects it.
			ClearSelection();
		}
		else
		{
			SelectedWinchSide = EWinchSide::Location;
			WinchPropertyPath = FComponentPropertyPath(Winch);
		}
		return true;
	}
	else if (HWinchDirectionProxy* DirectionProxy = HitProxyCast<HWinchDirectionProxy>(VisProxy))
	{
		if (SelectedWinchSide == EWinchSide::Rotation)
		{
			//Clicking a selected handle deselects it.
			ClearSelection();
		}
		else
		{
			SelectedWinchSide = EWinchSide::Rotation;
			WinchPropertyPath = FComponentPropertyPath(Winch);
		}
		return true;
	}

	return false;
}

bool FAGX_WireWinchVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!HasValidWinchSelection())
	{
		return false;
	}

	const UAGX_WireWinchComponent* WinchComponent = GetSelectedWinch();
	const FTransform& LocalToWorld = WinchComponent->GetComponentTransform();
	const FAGX_WireWinch& Winch = WinchComponent->WireWinch;
	if (SelectedWinchSide == EWinchSide::Location)
	{
		const FVector LocalLocation = Winch.Location;
		const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
		OutLocation = WorldLocation;
		return true;
	}
	else if (SelectedWinchSide == EWinchSide::Rotation)
	{
		const FVector LocalLocation = Winch.Location;
		const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
		const FRotator Rotation = Winch.Rotation;
		const FVector LocalDirection = Rotation.RotateVector(FVector::ForwardVector);
		const FVector WorldDirection = LocalToWorld.TransformVector(LocalDirection);
		const FVector WorldEndLocation = WorldLocation + (WorldDirection * 100.0f);
		OutLocation = WorldEndLocation;
		return true;
	}
	else
	{
		return false;
	}
}

bool FAGX_WireWinchVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	return false;
}

bool FAGX_WireWinchVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return false;
}

void FAGX_WireWinchVisualizer::EndEditing()
{
	ClearSelection();
}

bool FAGX_WireWinchVisualizer::HasValidWinchSelection() const
{
	return GetSelectedWinch() != nullptr && SelectedWinchSide != EWinchSide::None;
}

UAGX_WireWinchComponent* FAGX_WireWinchVisualizer::GetSelectedWinch()
{
	return Cast<UAGX_WireWinchComponent>(WinchPropertyPath.GetComponent());
}

const UAGX_WireWinchComponent* FAGX_WireWinchVisualizer::GetSelectedWinch() const
{
	return Cast<const UAGX_WireWinchComponent>(WinchPropertyPath.GetComponent());
}

void FAGX_WireWinchVisualizer::ClearSelection()
{
	SelectedWinchSide = EWinchSide::None;
	WinchPropertyPath.Reset();
}
