#include "Constraints/AGX_BallConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_BallConstraintComponent::UAGX_BallConstraintComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_3})
{
}

UAGX_BallConstraintComponent::~UAGX_BallConstraintComponent()
{
}

void UAGX_BallConstraintComponent::CreateNativeImpl()
{
	NativeBarrier.Reset(new FBallJointBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
