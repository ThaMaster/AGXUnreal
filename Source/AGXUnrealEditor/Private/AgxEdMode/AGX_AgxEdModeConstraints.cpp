// Fill out your copyright notice in the Description page of Project Settings.

#include "AgxEdMode/AGX_AgxEdModeConstraints.h"

#include "Utilities/AGX_EditorUtilities.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintFrameActor.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeConstraints"

UAGX_AgxEdModeConstraints* UAGX_AgxEdModeConstraints::GetInstance()
{
	static UAGX_AgxEdModeConstraints* ConstraintTool = nullptr;

	if (ConstraintTool == nullptr)
	{
		ConstraintTool = GetMutableDefault<UAGX_AgxEdModeConstraints>();
	}

	return ConstraintTool;
}

FText UAGX_AgxEdModeConstraints::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Constraints");
}

FText UAGX_AgxEdModeConstraints::GetTooltip() const
{
	return LOCTEXT("Tooltip", "Contains tools to quickly create and manage AGX Constraints");
}

AAGX_Constraint* UAGX_AgxEdModeConstraints::CreateConstraint() const
{
	if (!RigidBodyActor1.IsValid())
	{
		FAGX_EditorUtilities::ShowDialogBox(LOCTEXT(
			"CreateConstraintFailedNoActorOne",
			"Cannot create constraint. At least the first Rigid Body Actor must be chosen!"));
		return nullptr;
	}

	AAGX_Constraint* Constraint = FAGX_EditorUtilities::CreateConstraint(
		ConstraintType, RigidBodyActor1.Get(), RigidBodyActor2.Get(),
		/*Select*/ false, /*ShowNotification*/ true, /*InPlayingWorldIfAvailable*/ true);

	if (Constraint)
	{
		// Set constraint actor parent in scene hierarchy, and transform.
		switch (ConstraintParent)
		{
			case EAGX_ConstraintActorParent::RigidBodyActor1:
			{
				Constraint->AttachToActor(
					RigidBodyActor1.Get(), FAttachmentTransformRules::KeepRelativeTransform);
				// Transform is implicitly same as RigidBodyActor1, because of no relative
				// transform.
				break;
			}
			case EAGX_ConstraintActorParent::RigidBodyActor2:
			{
				if (RigidBodyActor2.IsValid())
				{
					Constraint->AttachToActor(
						RigidBodyActor2.Get(), FAttachmentTransformRules::KeepRelativeTransform);
					// Transform is implicitly same as RigidBodyActor2, because of no relative
					// transform.
				}
				else
				{
					// Transform of Rigid Body Actor 1 is usually a good starting point.
					Constraint->SetActorTransform(RigidBodyActor1.Get()->GetActorTransform());
				}
				break;
			}
			case EAGX_ConstraintActorParent::None:
			default:
			{
				// Transform of Rigid Body Actor 1 is usually a good starting point.
				Constraint->SetActorTransform(RigidBodyActor1.Get()->GetActorTransform());
				break;
			}
		};

		// Setup constraint attachment frames.
		AActor* FrameActor1 = nullptr;
		AActor* FrameActor2 = nullptr;

		switch (AttachmentFrameSource)
		{
			case EAGX_ConstraintFrameSource::ConstraintTransform:
			{
				FrameActor1 = FrameActor2 = Constraint;
				break;
			}
			case EAGX_ConstraintFrameSource::OneSharedFrameActor:
			{
				FrameActor1 = FrameActor2 =
					FAGX_EditorUtilities::CreateConstraintFrameActor(nullptr, false, true, true);
				FrameActor1->SetActorTransform(
					RigidBodyActor1.Get()->GetActorTransform()); // usually a good starting point
				break;
			}
			case EAGX_ConstraintFrameSource::TwoFrameActors:
			{
				FrameActor1 = FAGX_EditorUtilities::CreateConstraintFrameActor(
					RigidBodyActor1.Get(), false, true, true);
				FrameActor2 = FAGX_EditorUtilities::CreateConstraintFrameActor(
					RigidBodyActor2.Get(), false, true, true);
				break;
			}
			case EAGX_ConstraintFrameSource::RigidBodyActor1:
			{
				FrameActor1 = FrameActor2 = RigidBodyActor1.Get();
				break;
			}
			case EAGX_ConstraintFrameSource::RigidBodyActor2:
			{
				FrameActor1 = FrameActor2 = RigidBodyActor2.Get();
				break;
			}
			case EAGX_ConstraintFrameSource::LocalOnly:
			default:
			{
				break;
			}
		};

		Constraint->BodyAttachment1.FrameDefiningActor = FrameActor1;
		Constraint->BodyAttachment2.FrameDefiningActor = FrameActor2;

		Constraint->BodyAttachment1.OnFrameDefiningActorChanged(Constraint);
		Constraint->BodyAttachment2.OnFrameDefiningActorChanged(Constraint);

		/// \todo If in-game, we need to create the constraint in a deferred way so that frame
		/// actors are set before the constraint has been finished!
	}

	FAGX_EditorUtilities::SelectActor(Constraint);

	return Constraint;
}

#undef LOCTEXT_NAMESPACE
