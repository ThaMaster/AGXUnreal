#include "Constraints/AGX_HingeConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/HingeBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_HingeConstraintComponent::UAGX_HingeConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3, EDofFlag::DofFlagRotational1,
		   EDofFlag::DofFlagRotational2},
		  /*bbIsSecondaryConstraintRotational*/ true)
{
}

UAGX_HingeConstraintComponent::~UAGX_HingeConstraintComponent()
{
}

void UAGX_HingeConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FHingeBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
