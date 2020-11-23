#include "Constraints/AGX_PrismaticConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/PrismaticBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_PrismaticConstraintComponent::UAGX_PrismaticConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DOF_FLAG_ROTATIONAL_1, EDofFlag::DOF_FLAG_ROTATIONAL_2,
		   EDofFlag::DOF_FLAG_ROTATIONAL_3, EDofFlag::DOF_FLAG_TRANSLATIONAL_1,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_2},
		  /*bIsSecondaryConstraintRotational*/ false)
{
}

UAGX_PrismaticConstraintComponent::~UAGX_PrismaticConstraintComponent()
{
}

void UAGX_PrismaticConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FPrismaticBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
