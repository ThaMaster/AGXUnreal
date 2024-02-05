// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSensorComponentVisualizer"

void FAGX_LidarSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_LidarSensorComponent* Lidar = Cast<const UAGX_LidarSensorComponent>(Component);
	if (Lidar == nullptr || !Lidar->ShouldRender())
		return;

	static constexpr FColor Color {243, 139, 0};
	static constexpr float Radius {10.f};

	DrawWireCylinder(
		PDI, Lidar->GetComponentLocation(), Lidar->GetForwardVector(), Lidar->GetRightVector(),
		Lidar->GetUpVector(), Color, Radius, Radius / 2.f, 32, SDPG_Foreground);
}

#undef LOCTEXT_NAMESPACE
