#include "Constraints/AGX_CylindricalConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/CylindricalJointBarrier.h"
#include "AGX_LogCategory.h"

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

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cylindrical constraint %s: could not get Rigid Body Actor from Body Attachment "
				 "1. Constraint cannot be created."),
			*GetFName().ToString());
		return;
	}

	if (BodyAttachment2.GetRigidBody() != nullptr && RigidBody2 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cylindrical constraint %s: could not get Rigid Body Actor from Body Attachment "
				 "2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocationFromBody();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocationFromBody();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotationFromBody();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotationFromBody();

	// Ok if second is nullptr, means that the first body is constrainted to the world.
	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2,
		&FrameRotation2);
}
