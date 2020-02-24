#include "Constraints/AGX_BallConstraintComponent.h"

#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

UAGX_BallConstraintComponent::UAGX_BallConstraintComponent()
	: UAGX_ConstraintComponent({EDofFlag::DOF_FLAG_TRANSLATIONAL_1,
								EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
								EDofFlag::DOF_FLAG_TRANSLATIONAL_3})
{
}

UAGX_BallConstraintComponent::~UAGX_BallConstraintComponent()
{
}

void UAGX_BallConstraintComponent::CreateNativeImpl()
{
	NativeBarrier.Reset(new FBallJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Ball constraint %s: could not get Rigid Body from Body Attachment 1. "
				 "Constraint cannot be created."),
			*GetFName().ToString());
		return;
	}

	if (BodyAttachment2.GetRigidBodyComponent() != nullptr && RigidBody2 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Ball constraint %s: could not get Rigid Body from Body Attachment 2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	// Ok if second body is nullptr, means that the first body is constrained
	// to the world.
	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2, &FrameRotation2);
}
