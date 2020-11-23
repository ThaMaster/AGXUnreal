#include "Constraints/AGX_HingeConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/HingeBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_HingeConstraintComponent::UAGX_HingeConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_3, EDofFlag::DOF_FLAG_ROTATIONAL_1,
		   EDofFlag::DOF_FLAG_ROTATIONAL_2},
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
