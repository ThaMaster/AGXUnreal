#include "Constraints/AGX_LockConstraintComponent.h"

#include "Constraints/LockJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_LockConstraintComponent::UAGX_LockConstraintComponent()
	: UAGX_ConstraintComponent({EDofFlag::DofFlagTranslational1,
								EDofFlag::DofFlagTranslational2,
								EDofFlag::DofFlagTranslational3, EDofFlag::DofFlagRotational1,
								EDofFlag::DofFlagRotational2, EDofFlag::DofFlagRotational3})
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
