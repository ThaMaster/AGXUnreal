#include "Tires/AGX_TireComponentVisualizer.h"

// AGXUnreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

// Unreal Engine inclues.
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_TireComponentVisualizer"

void FAGX_TireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TwoBodyTireComponent* Tire = Cast<const UAGX_TwoBodyTireComponent>(Component);
	if (Tire == nullptr)
		return;

	DrawTwoBodyTire(Tire, View, PDI);
}

#undef LOCTEXT_NAMESPACE
