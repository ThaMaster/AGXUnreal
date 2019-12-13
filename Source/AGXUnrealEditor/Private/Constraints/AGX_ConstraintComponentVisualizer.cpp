// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_ConstraintComponentVisualizer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "SceneManagement.h"

#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintDofGraphicsComponent.h"

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

	const AAGX_Constraint* Constraint =
		Cast<const AAGX_Constraint>(ConstraintComponent->GetOwner());

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
	const AAGX_Constraint* Constraint = Cast<const AAGX_Constraint>(Component->GetOwner());

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

void FAGX_ConstraintComponentVisualizer::DrawConstraint(
	const AAGX_Constraint* Constraint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (Constraint == nullptr)
		return;

	for (int RigidBodyIndex = 0; RigidBodyIndex < 2; ++RigidBodyIndex)
	{
		FAGX_ConstraintBodyAttachment BodyAttachment =
			RigidBodyIndex == 0 ? Constraint->BodyAttachment1 : Constraint->BodyAttachment2;

		if (AActor* RigidBodyActor = BodyAttachment.RigidBodyActor)
		{
			// Highlight Rigid Body Actor
			if (bHighlightUsingBoundingBox)
			{
				FBox LocalAABB = RigidBodyActor->CalculateComponentsBoundingBoxInLocalSpace(
					/*bNonColliding*/ true);

				DrawOrientedWireBox(
					PDI, RigidBodyActor->GetActorLocation(),
					RigidBodyActor->GetActorForwardVector(), RigidBodyActor->GetActorRightVector(),
					RigidBodyActor->GetActorUpVector(), LocalAABB.GetExtent(), HighlightColor,
					SDPG_World, HighlightThickness, /*DepthBias*/ 0.0f,
					/*bScreenSpace*/ true);
			}
			else if (bHighlightUsingCircle)
			{
				FVector Direction =
					(RigidBodyActor->GetActorLocation() - View->ViewLocation).GetSafeNormal();
				float Distance = 40.0f;
				FVector Location = View->ViewLocation + Direction * Distance;
				float Radius = GetWorldSizeFromScreenFactor(
					0.08f, FMath::DegreesToRadians(View->FOV), Distance);

				DrawCircle(
					PDI, Location, View->GetViewRight(), View->GetViewUp(), HighlightColor, Radius,
					/*Sides*/ 32, SDPG_Foreground, HighlightThickness, /*DepthBias*/ 0.0f,
					/*bScreenSpace*/ true);
			}

			// Draw tripod for final Attachment Frame
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

				DrawCoordinateSystemAxes(
					PDI, BodyAttachment.GetGlobalFrameLocation(),
					BodyAttachment.GetGlobalFrameRotation().Rotator(), FrameGizmoScale,
					SDPG_Foreground, FrameGizmoThickness, /*DepthBias*/ 0.0f,
					/*bScreenSpace*/ true);
			}
		}
	}

	if (bDrawLineBetweenActors)
	{
		AActor* RigidBodyActor1 = Constraint->BodyAttachment1.RigidBodyActor;
		AActor* RigidBodyActor2 = Constraint->BodyAttachment2.RigidBodyActor;

		if (RigidBodyActor1 && RigidBodyActor2)
		{
			float Distance = 100.0f;

			FVector Direction1 =
				(RigidBodyActor1->GetActorLocation() - View->ViewLocation).GetSafeNormal();
			FVector Location1 = View->ViewLocation + Direction1 * Distance;

			FVector Direction2 =
				(RigidBodyActor2->GetActorLocation() - View->ViewLocation).GetSafeNormal();
			FVector Location2 = View->ViewLocation + Direction2 * Distance;

			DrawDashedLine(
				PDI, Location1, Location2, HighlightColor, HighlightThickness, SDPG_Foreground,
				/*DepthBias*/ 0.0f);
		}
	}
}

void FAGX_ConstraintComponentVisualizer::DrawConstraintHUD(
	const AAGX_Constraint* Constraint, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	if (Constraint->AreFramesInViolatedState())
	{
		FVector2D Position(0.45f, 0.35f);
		FText Text = FText::FromString("Constraint Frames In Violated State!");
		UFont* Font = GEngine->GetSubtitleFont();
		FCanvasTextItem CanvasText(
			Position * Canvas->GetViewRect().Size(), Text, Font, FColor::Red);

		Canvas->DrawItem(CanvasText);

		// GEngine->AddOnScreenDebugMessage(45456, 5.f, FColor::Red, TEXT("Constraint Frames In
		// Violated State!"));
	}
}

#undef LOCTEXT_NAMESPACE
