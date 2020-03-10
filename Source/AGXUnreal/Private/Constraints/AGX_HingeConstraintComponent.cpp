#include "Constraints/AGX_HingeConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/HingeBarrier.h"

// Unreal Engine includes.
#include "AGX_LogCategory.h"

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

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Hinge constraint %s in %s: could not get Rigid Body from Body Attachment 1. Constraint "
				 "cannot be created."),
			*GetFName().ToString(), *GetOwner()->GetFName().ToString());
		return;
	}

	if (BodyAttachment2.GetRigidBody() != nullptr && RigidBody2 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Hinge constraint %s: could not get Rigid Body from Body Attachment 2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	// Ok if second is nullptr, means that the first body is constrained to the
	// world.
	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2, &FrameRotation2);
}
