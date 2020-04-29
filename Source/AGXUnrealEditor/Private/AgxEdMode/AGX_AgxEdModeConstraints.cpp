#include "AgxEdMode/AGX_AgxEdModeConstraints.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
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

AAGX_ConstraintActor* UAGX_AgxEdModeConstraints::CreateConstraint() const
{
	if (RigidBody1.GetRigidBody() == nullptr)
	{
		FAGX_EditorUtilities::ShowDialogBox(LOCTEXT(
			"CreateConstraintFailedNoActorOne",
			"Cannot create constraints. At least the first Rigid Body must be chosen!"));
		return nullptr;
	}

	UAGX_RigidBodyComponent* Body1 = RigidBody1.GetRigidBody();
	UAGX_RigidBodyComponent* Body2 = RigidBody2.GetRigidBody();
	AAGX_ConstraintActor* Constraint = FAGX_EditorUtilities::CreateConstraintActor(
		ConstraintType, Body1, Body2, false, true, true);

	if (Constraint)
	{
		/// \todo Consider using the UAGX_RigidBodyComponent's transform in the below,
		/// instead of the owning Actor's transform.

		// Set constraint actor parent in scene hierarchy, and transform.
		switch (ConstraintParent)
		{
			case EAGX_ConstraintActorParent::RigidBodyActor1:
			{
				Constraint->AttachToActor(
					RigidBody1.GetOwningActor(), FAttachmentTransformRules::KeepRelativeTransform);
				// Transform is implicitly same as RigidBodyActor1, because of no relative
				// transform.
				break;
			}
			case EAGX_ConstraintActorParent::RigidBodyActor2:
			{
				if (RigidBody2.GetRigidBody() != nullptr)
				{
					Constraint->AttachToActor(
						RigidBody2.GetOwningActor(), FAttachmentTransformRules::KeepRelativeTransform);
					// Transform is implicitly same as RigidBody2, because of no relative transform.
				}
				else
				{
					// Transform of rigid body 1 is usually a good starting points.
					Constraint->SetActorTransform(RigidBody1.OwningActor->GetActorTransform());
				}
				break;
			}
			case EAGX_ConstraintActorParent::None:
				// Deliberate fallthrough.
			default:
			{
				// Transform of Rigid Body Actor 1 is usually a good starting point.
				Constraint->SetActorTransform(RigidBody1.OwningActor->GetActorTransform());
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
				// Usually a good starting point.
				FrameActor1->SetActorTransform(RigidBody1.OwningActor->GetActorTransform());
				break;
			}
			case EAGX_ConstraintFrameSource::TwoFrameActors:
			{
				FrameActor1 = FAGX_EditorUtilities::CreateConstraintFrameActor(
							RigidBody1.GetOwningActor(), false, true, true);
				FrameActor2 = FAGX_EditorUtilities::CreateConstraintFrameActor(
							RigidBody2.GetOwningActor(), false, true, true);
				break;
			}
			case EAGX_ConstraintFrameSource::RigidBodyActor1:
			{
				FrameActor1 = FrameActor2 = RigidBody1.GetOwningActor();
				break;
			}
			case EAGX_ConstraintFrameSource::RigidBodyActor2:
			{
				FrameActor1 = FrameActor2 = RigidBody2.GetOwningActor();
				break;
			}
			case EAGX_ConstraintFrameSource::LocalOnly:
				// Deliberate fallthrough.
			default:
			{
				break;
			}
		};

		UAGX_ConstraintComponent* ConstraintComponent = Constraint->GetConstraintComponent();
#if AGXUNREAL_FRAME_DEFINING_TYPE == AGXUNREAL_COMPONENT
		ConstraintComponent->BodyAttachment1.FrameDefiningComponent.OwningActor = FrameActor1;
		ConstraintComponent->BodyAttachment2.FrameDefiningComponent.OwningActor = FrameActor2;

		ConstraintComponent->BodyAttachment1.OnFrameDefiningComponentChanged(ConstraintComponent);
		ConstraintComponent->BodyAttachment1.OnFrameDefiningComponentChanged(ConstraintComponent);
#elif AGXUNREAL_FRAME_DEFINING_TYPE == AGXUNREAL_ACTOR
		ConstraintComponent->BodyAttachment1.FrameDefiningActor = FrameActor1;
		ConstraintComponent->BodyAttachment2.FrameDefiningActor = FrameActor2;

		ConstraintComponent->BodyAttachment1.OnFrameDefiningActorChanged(ConstraintComponent);
		ConstraintComponent->BodyAttachment2.OnFrameDefiningActorChanged(ConstraintComponent);
#endif
		/// \todo If in-game, we need to create the constraint in a deferred way so that frame
		/// actors are set before the constraint has been finished!
	}

	FAGX_EditorUtilities::SelectActor(Constraint);

	return Constraint;
}

#undef LOCTEXT_NAMESPACE
