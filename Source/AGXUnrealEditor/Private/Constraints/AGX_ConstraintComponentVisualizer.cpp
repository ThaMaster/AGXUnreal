#include "Constraints/AGX_ConstraintComponentVisualizer.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintDofGraphicsComponent.h"

// Unreal Engine inclues.
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintComponentVisualizer"

namespace
{
	/// \todo Change these to settings editable from editor UI!

	const bool bHighlightUsingBoundingBox = false;
	const bool bHighlightUsingCircle = true;
	const FColor HighlightColor(243, 139, 0);
	const float HighlightThickness(1.0f);

	const bool bDrawAttachmenFrameTripod = true;
	const float FrameGizmoScale(50.0f);
	const float FrameGizmoThickness(1.0f);

	const bool bDrawLineBetweenActors = true;

	void DrawCoordinateSystemAxes(
		FPrimitiveDrawInterface* PDI, FVector const& AxisLoc, FRotator const& AxisRot, float Scale,
		uint8 DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
	{
		FRotationMatrix R(AxisRot);
		FVector const X = R.GetScaledAxis(EAxis::X);
		FVector const Y = R.GetScaledAxis(EAxis::Y);
		FVector const Z = R.GetScaledAxis(EAxis::Z);

		PDI->DrawLine(
			AxisLoc, AxisLoc + X * Scale, FLinearColor::Red, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Y * Scale, FLinearColor::Green, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Z * Scale, FLinearColor::Blue, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
	}

	static FVector2D operator*(const FVector& NormalizedCoordinates, const FIntPoint& ViewSize)
	{
		return FVector2D(
			NormalizedCoordinates.X * ViewSize.X, NormalizedCoordinates.Y * ViewSize.Y);
	}
}

void FAGX_ConstraintComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintComponent* ConstraintComponent =
		Cast<const UAGX_ConstraintComponent>(Component);

	if (ConstraintComponent == nullptr)
		return;

	const UAGX_ConstraintComponent* Constraint =
		Cast<const UAGX_ConstraintComponent>(ConstraintComponent);

	DrawConstraint(Constraint, View, PDI);

	if (UAGX_ConstraintDofGraphicsComponent* DofGraphics = Constraint->GetDofGraphics())
	{
		/// Hack to force update of render transform, if for example the constraint uses a
		/// constraint transform actor and it was moved without UAGX_ConstraintDofGraphicsComponent
		/// knowing about it... \todo Might be a better way to do this?
		DofGraphics->OnBecameSelected();
	}
}

void FAGX_ConstraintComponentVisualizer::DrawVisualizationHUD(
	const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	const UAGX_ConstraintComponent* Constraint = Cast<const UAGX_ConstraintComponent>(Component);

	if (!Constraint)
	{
		return;
	}

	DrawConstraintHUD(Constraint, Viewport, View, Canvas);
}

float GetScreenToWorldFactor(float FOV, float WorldDistance)
{
	float Hack = 0.5f; // because result seemed a bit off...
	return Hack * 2.0f * WorldDistance * FMath::Atan(FOV / 2.0f);
}

float GetWorldSizeFromScreenFactor(float ScreenFactor, float FOV, float WorldDistance)
{
	return ScreenFactor * GetScreenToWorldFactor(FOV, WorldDistance);
}

float GetScreenFactorFromWorldSize(float WorldSize, float FOV, float WorldDistance)
{
	return WorldSize / GetScreenToWorldFactor(FOV, WorldDistance);
}

namespace
{
	FBox GetBoundingBox(UAGX_RigidBodyComponent* Body)
	{
		FBox Box(ForceInit);

		const FTransform& BodyToWorld = Body->GetComponentTransform();
		const FTransform WorldToBody = BodyToWorld.Inverse();

		TArray<USceneComponent*> Children;
		Body->GetChildrenComponents(true, Children);
		for (auto Child : Children)
		{
			const FTransform ComponentToBody = Child->GetComponentTransform() * WorldToBody;
			const FBoxSphereBounds BoundInBodySpace = Child->CalcBounds(ComponentToBody);
			Box += BoundInBodySpace.GetBox();
		}
		return Box;
	}
}

/// \todo Consider moving GetRigidBody with fallback to FAGX_ConstraintBodyAttachment.
UAGX_RigidBodyComponent* GetRigidBody(
	const FAGX_RigidBodyReference& BodyReference, AActor* Fallback)
{
	if (UAGX_RigidBodyComponent* Body = BodyReference.GetRigidBody())
	{
		return Body;
	}

	if (Fallback == nullptr)
	{
		return nullptr;
	}

	TArray<UAGX_RigidBodyComponent*> AllBodies;
	Fallback->GetComponents(AllBodies, BodyReference.bSearchChildActors);
	for (UAGX_RigidBodyComponent* Candidate : AllBodies)
	{
		if (Candidate->GetFName() == BodyReference.BodyName)
		{
			return Candidate;
		}
	}

	return nullptr;
}

void RenderBodyMarker(
	UAGX_RigidBodyComponent* Body, float CircleScreenFactor, const FColor& Color,
	const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (bHighlightUsingBoundingBox)
	{
		FBox LocalAABB = GetBoundingBox(Body);

		DrawOrientedWireBox(
			PDI, Body->GetComponentLocation(), Body->GetForwardVector(), Body->GetRightVector(),
			Body->GetUpVector(), LocalAABB.GetExtent(), Color, SDPG_World, HighlightThickness,
			/*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
	}

	if (bHighlightUsingCircle)
	{
		const FVector Direction =
			(Body->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
		const float Distance = 40.0f;
		const FVector Location = View->ViewLocation + Direction * Distance;
		const float Radius = GetWorldSizeFromScreenFactor(
			CircleScreenFactor, FMath::DegreesToRadians(View->FOV), Distance);
		DrawCircle(
			PDI, Location, View->GetViewRight(), View->GetViewUp(), Color, Radius, 32,
			SDPG_Foreground, HighlightThickness, /*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
	}

	if (bDrawAttachmenFrameTripod)
	{
		// It is important to make sure that drawn coordinate system does not interfere
		// with the default transform gizmo. Therefore, make sure thickness of drawn
		// coordinate system is thinner than transform gizmo, so that the transform gizmo is
		// always selectable.
		//
		// If is not thinner, in the scenario where the coordinate system equals the
		// currently active transform gizmo (i.e. visually overlapping), and they are
		// located inside a mesh while the camera is ouside of the mesh, there are
		// difficulties	selecting the transform gizmo (even if HitProxy, depth bias, etc are
		// used).

/// \todo This doesn't work in the Blueprint editor as RigidBodyReference is currently implemented
/// RigidBodyReference must be made aware of fallback owneres.
/// Can I just set the OwningActor to the Constraint's owner? What would be cool.
/// What will the DeatailCustomization do?
#if 0
		DrawCoordinateSystemAxes(
			PDI, BodyAttachment.GetGlobalFrameLocation(),
			BodyAttachment.GetGlobalFrameRotation().Rotator(), FrameGizmoScale, SDPG_Foreground,
			FrameGizmoThickness, /*DepthBias*/ 0.0f,
			/*bScreenSpace*/ true);
#endif
	}
}

void FAGX_ConstraintComponentVisualizer::DrawConstraint(
	const UAGX_ConstraintComponent* Constraint, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	if (Constraint == nullptr)
		return;

	const FAGX_RigidBodyReference& BodyReference1 = Constraint->BodyAttachment1.RigidBody;
	const FAGX_RigidBodyReference& BodyReference2 = Constraint->BodyAttachment2.RigidBody;

	AActor* BodyOwnerFallback = Constraint->GetOwner();
	UAGX_RigidBodyComponent* Body1 = GetRigidBody(BodyReference1, BodyOwnerFallback);
	UAGX_RigidBodyComponent* Body2 = GetRigidBody(BodyReference2, BodyOwnerFallback);

	if (Body1 != nullptr)
	{
		float CircleScreenFactor = 0.08f;
		RenderBodyMarker(Body1, CircleScreenFactor,HighlightColor, View, PDI);
	}
	if (Body2 != nullptr)
	{
		float CircleScreenFactor = 0.06f;
		FColor Color = FColor(
			HighlightColor.R * 0.6f, HighlightColor.G * 0.6f, HighlightColor.B * 0.6f,
			HighlightColor.A);
		RenderBodyMarker(Body2, CircleScreenFactor, Color, View, PDI);
	}

	if (bDrawLineBetweenActors && Body1 != nullptr && Body2 != nullptr)
	{
		float Distance = 100.0f;

		FVector Direction1 = (Body1->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
		FVector Location1 = View->ViewLocation + Direction1 * Distance;

		FVector Direction2 = (Body2->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
		FVector Location2 = View->ViewLocation + Direction2 * Distance;

		DrawDashedLine(
			PDI, Location1, Location2, HighlightColor, HighlightThickness, SDPG_Foreground,
			/*DepthBias*/ 0.0f);
	}
}

void FAGX_ConstraintComponentVisualizer::DrawConstraintHUD(
	const UAGX_ConstraintComponent* Constraint, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	FString Message;
	if (Constraint->AreFramesInViolatedState(KINDA_SMALL_NUMBER, &Message))
	{
		FVector2D Position(0.45f, 0.35f);
		FText Text = FText::FromString(
			FString::Printf(TEXT("Constraint Frames In Violated State!\n%s"), *Message));
		UFont* Font = GEngine->GetSubtitleFont();
		FCanvasTextItem CanvasText(
			Position * Canvas->GetViewRect().Size(), Text, Font, FColor::Red);

		Canvas->DrawItem(CanvasText);

		// GEngine->AddOnScreenDebugMessage(45456, 5.f, FColor::Red, TEXT("Constraint Frames In
		// Violated State!"));
	}
}

#undef LOCTEXT_NAMESPACE
