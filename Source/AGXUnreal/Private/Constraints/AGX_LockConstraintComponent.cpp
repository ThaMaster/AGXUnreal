#include "Constraints/AGX_LockConstraintComponent.h"

#include "Constraints/LockJointBarrier.h"
#include "AGX_LogCategory.h"

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

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (!RigidBody1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Lock constraint %s: could not get Rigid Body Actor from Body Attachment 1."),
			*GetFName().ToString());
		return;
	}

	if ((BodyAttachment2.RigidBodyActor && !RigidBody2))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Lock constraint %s: could not get Rigid Body Actor from Body Attachment 2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2,
		&FrameRotation2); // ok if second is nullptr
}
