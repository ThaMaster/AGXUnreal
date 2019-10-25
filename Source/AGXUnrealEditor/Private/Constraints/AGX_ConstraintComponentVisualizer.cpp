// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_ConstraintComponentVisualizer.h"

#include "SceneManagement.h"

#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintComponent.h"


#define LOCTEXT_NAMESPACE "FAGX_ConstraintComponentVisualizer"


static const FColor	RigidBodyHighlightColor(243, 139, 0);
static const float RigidBodyHighlightThickness(3.0f);
static const float FrameGizmoScale(70.0f);
static const float FrameGizmoThickness(5.0f);


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
				DrawCoordinateSystem(PDI,
					BodyAttachment.GetGlobalFrameLocation(),
					BodyAttachment.GetGlobalFrameRotation().Rotator(),
					FrameGizmoScale, SDPG_Foreground, FrameGizmoThickness);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
