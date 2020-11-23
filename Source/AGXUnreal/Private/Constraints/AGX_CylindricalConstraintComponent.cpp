#include "Constraints/AGX_CylindricalConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/CylindricalJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_CylindricalConstraintComponent::UAGX_CylindricalConstraintComponent()
	: UAGX_Constraint2DofComponent(
		  {EDofFlag::DOF_FLAG_ROTATIONAL_1, EDofFlag::DOF_FLAG_ROTATIONAL_2,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2},
		  /*bIsSecondaryConstraint1Rotational*/ false,
		  /*bIsSecondaryConstraint2Rotational*/ true)
{
}

UAGX_CylindricalConstraintComponent::~UAGX_CylindricalConstraintComponent()
{
}

void UAGX_CylindricalConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FCylindricalJointBarrier());
	
	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
