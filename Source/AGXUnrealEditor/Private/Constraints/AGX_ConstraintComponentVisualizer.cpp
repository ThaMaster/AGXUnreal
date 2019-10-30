// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_ConstraintComponentVisualizer.h"

#include "SceneManagement.h"

#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintComponent.h"


#define LOCTEXT_NAMESPACE "FAGX_ConstraintComponentVisualizer"


namespace
{
	const FColor RigidBodyHighlightColor(243, 139, 0);
	const float RigidBodyHighlightThickness(3.0f);
	const float FrameGizmoScale(70.0f);
	const float FrameGizmoThickness(3.0f);


	void DrawCoordinateSystemAxes(FPrimitiveDrawInterface* PDI, FVector const& AxisLoc, FRotator const& AxisRot, float Scale, uint8 DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
	{
		FRotationMatrix R(AxisRot);
		FVector const X = R.GetScaledAxis(EAxis::X);
		FVector const Y = R.GetScaledAxis(EAxis::Y);
		FVector const Z = R.GetScaledAxis(EAxis::Z);

		PDI->DrawLine(AxisLoc, AxisLoc + X * Scale, FLinearColor::Red, DepthPriority, Thickness, DepthBias, bScreenSpace);
		PDI->DrawLine(AxisLoc, AxisLoc + Y * Scale, FLinearColor::Green, DepthPriority, Thickness, DepthBias, bScreenSpace);
		PDI->DrawLine(AxisLoc, AxisLoc + Z * Scale, FLinearColor::Blue, DepthPriority, Thickness, DepthBias, bScreenSpace);
	}
}


void FAGX_ConstraintComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintComponent* ConstraintComponent = Cast<const UAGX_ConstraintComponent>(Component);

	if (ConstraintComponent == nullptr)
		return;

	const AAGX_Constraint* Constraint = Cast<const AAGX_Constraint>(ConstraintComponent->GetOwner());

	DrawConstraint(Constraint, View, PDI);
}


void FAGX_ConstraintComponentVisualizer::DrawConstraint(const AAGX_Constraint* Constraint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (Constraint == nullptr)
		return;

	AActor* RigidBodyActor1 = Constraint->BodyAttachment1.RigidBodyActor;
	AActor* RigidBodyActor2 = Constraint->BodyAttachment2.RigidBodyActor;

	for (int RigidBodyIndex = 0; RigidBodyIndex < 2; ++RigidBodyIndex)
	{
		FAGX_ConstraintBodyAttachment BodyAttachment = RigidBodyIndex == 0 ?
			Constraint->BodyAttachment1 :
			Constraint->BodyAttachment2;

		if (BodyAttachment.RigidBodyActor)
		{
			// Highlight Rigid Body Actor
			{
				FBox AABB = BodyAttachment.RigidBodyActor->GetComponentsBoundingBox(/*bNonColliding*/ true);

				DrawWireBox(PDI,
					AABB, RigidBodyHighlightColor, SDPG_World,
					RigidBodyHighlightThickness, /*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
			}

			// Draw gizmo for final Attachment Frame
			{
				// It is important to make sure that drawn coordinate system does not interfere
				// with the default transform gizmo. Therefore, make sure thickness of drawn coordinate
				// system is thinner than transform gizmo, so that the transform gizmo is always selectable.
				// 
				// If is not thinner, in the scenario where the coordinate system equals the currently active
				// transform gizmo (i.e. visually overlapping), and they are located inside a mesh while the
				// camera is ouside of the mesh, there are difficulties	selecting the transform gizmo (even if
				// HitProxy, depth bias, etc are used). 
				
				DrawCoordinateSystemAxes(PDI,
					BodyAttachment.GetGlobalFrameLocation(),
					BodyAttachment.GetGlobalFrameRotation().Rotator(),
					FrameGizmoScale, SDPG_Foreground,
					FrameGizmoThickness, /*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
