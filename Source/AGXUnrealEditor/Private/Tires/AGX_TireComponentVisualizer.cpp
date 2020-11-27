#include "Tires/AGX_TireComponentVisualizer.h"

// AGXUnreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_TireComponentVisualizer"

namespace
{
	void DrawTirePrimitive(
		UAGX_RigidBodyComponent* RigidBody, float Radius, const FSceneView* View,
		FPrimitiveDrawInterface* PDI, FColor Color)
	{
		if (RigidBody == nullptr || Radius <= 0.f)
		{
			return;
		}

		constexpr int32 NUM_SIDES {64};
		const FTransform BodyTransform = RigidBody->GetComponentTransform();
		const float CylinderHalfHeight = 0.1f * Radius;

		// Note: The agx::Tire model is implemented in such a way that it is assumed that the axis of
		// rotation of the tire is along the y-axis of the tire RigidBody. Therefore we draw the
		// cylinder so that it's principal axis is aligned with the y-axis. We have to consider the
		// different coordinate systems used in AGX Dynamics vs Unreal; AGX Dynamics y-axis goes in
		// Unreal's negative y-axis direction, but since we are drawing a symmetric cylinder with no
		// offset from origo there is no difference in this case.
		DrawWireCylinder(
			PDI, BodyTransform.GetLocation(), BodyTransform.GetUnitAxis(EAxis::Z),
			BodyTransform.GetUnitAxis(EAxis::X), BodyTransform.GetUnitAxis(EAxis::Y), Color, Radius,
			CylinderHalfHeight,
			NUM_SIDES, SDPG_Foreground);
	}

	void DrawTwoBodyTire(
		const UAGX_TwoBodyTireComponent* Tire, const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		if (Tire == nullptr)
		{
			return;
		}

		const FColor TirePrimitiveColor(240, 230, 0);

		UAGX_RigidBodyComponent* HubRigidBody = Tire->GetHubRigidBody();
		UAGX_RigidBodyComponent* TireRigidBody = Tire->GetTireRigidBody();

		DrawTirePrimitive(HubRigidBody, Tire->InnerRadius, View, PDI, TirePrimitiveColor);
		DrawTirePrimitive(TireRigidBody, Tire->OuterRadius, View, PDI, TirePrimitiveColor);
	}
}

void FAGX_TireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TwoBodyTireComponent* Tire = Cast<const UAGX_TwoBodyTireComponent>(Component);
	if (Tire == nullptr)
		return;

	DrawTwoBodyTire(Tire, View, PDI);
}

#undef LOCTEXT_NAMESPACE
