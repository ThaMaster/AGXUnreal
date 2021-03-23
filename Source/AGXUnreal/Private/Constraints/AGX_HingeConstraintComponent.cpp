#include "Constraints/AGX_HingeConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/HingeBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "AGX_LogCategory.h"

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

FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_HingeConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FHingeBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName(), GetOwner()->GetFName());
}
