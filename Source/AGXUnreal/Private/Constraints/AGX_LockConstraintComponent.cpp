#include "Constraints/AGX_LockConstraintComponent.h"

#include "Constraints/LockJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_LockConstraintComponent::UAGX_LockConstraintComponent()
	: UAGX_ConstraintComponent({EDofFlag::DOF_FLAG_TRANSLATIONAL_1,
								EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
								EDofFlag::DOF_FLAG_TRANSLATIONAL_3, EDofFlag::DOF_FLAG_ROTATIONAL_1,
								EDofFlag::DOF_FLAG_ROTATIONAL_2, EDofFlag::DOF_FLAG_ROTATIONAL_3})
{
}

UAGX_LockConstraintComponent::~UAGX_LockConstraintComponent()
{
}

void UAGX_LockConstraintComponent::CreateNativeImpl()
{
	NativeBarrier.Reset(new FLockJointBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
