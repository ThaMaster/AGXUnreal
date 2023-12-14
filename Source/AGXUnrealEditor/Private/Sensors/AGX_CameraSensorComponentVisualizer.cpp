// Copyright 2023, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraSensorComponent.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraSensorComponentVisualizer"

void FAGX_CameraSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_CameraSensorComponent* Cam = Cast<const UAGX_CameraSensorComponent>(Component);
	if (Cam == nullptr || !Cam->ShouldRender())
		return;

	if (!UAGX_CameraSensorComponent::IsFovValid(Cam->FOV))
		return;

	if (!UAGX_CameraSensorComponent::IsResolutionValid(Cam->Resolution))
		return;

	// Todo: draw camera sensor here.
}

#undef LOCTEXT_NAMESPACE
