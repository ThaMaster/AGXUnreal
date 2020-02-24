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
	if (!RigidBodyActor1.IsValid())
	{
		FAGX_EditorUtilities::ShowDialogBox(LOCTEXT(
			"CreateConstraintFailedNoActorOne",
			"Cannot create constraint. At least the first Rigid Body Actor must be chosen!"));
		return nullptr;
	}

	/// \todo Figure out how to setup constraint creation so that we can pick a
	/// single UAGX_RigidBodyComponent from the selected Actors. There is very
	/// similar code in AGX_TopMenu.cpp.

	TArray<UAGX_RigidBodyComponent*> Bodies1 = UAGX_RigidBodyComponent::GetFromActor(RigidBodyActor1.Get());
	TArray<UAGX_RigidBodyComponent*> Bodies2 = UAGX_RigidBodyComponent::GetFromActor(RigidBodyActor2.Get());

	if (Bodies1.Num() != 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create constraint with actor '%s' because it doesn't contain exactly one body."),
			*RigidBodyActor1->GetName());
		return nullptr;
	}

	if (Bodies2.Num() != 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create constraint with actor '%s' because it doesn't contain exactly one body."),
			*RigidBodyActor2->GetName());
		return nullptr;
	}

	AAGX_ConstraintActor* Constraint = FAGX_EditorUtilities::CreateConstraintActor(
		ConstraintType, Bodies1[0], Bodies2[0],
		/*Select*/ false, /*ShowNotification*/ true, /*InPlayingWorldIfAvailable*/ true);

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
				// Deliberate fallthrough.
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
				// Deliberate fallthrough.
			default:
			{
				break;
			}
		};

		UAGX_ConstraintComponent* ConstraintComponent = Constraint->GetConstraintComponent();
		ConstraintComponent->BodyAttachment1.FrameDefiningActor = FrameActor1;
		ConstraintComponent->BodyAttachment2.FrameDefiningActor = FrameActor2;

		ConstraintComponent->BodyAttachment1.OnFrameDefiningActorChanged(ConstraintComponent);
		ConstraintComponent->BodyAttachment2.OnFrameDefiningActorChanged(ConstraintComponent);

		/// \todo If in-game, we need to create the constraint in a deferred way so that frame
		/// actors are set before the constraint has been finished!
	}

	FAGX_EditorUtilities::SelectActor(Constraint);

	return Constraint;
}

#undef LOCTEXT_NAMESPACE
