#include "Constraints/AGX_PrismaticConstraintComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/PrismaticBarrier.h"

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

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Prismatic constraint %s: could not get Rigid Body Actor from Body Attachment 1. "
				 "Constraint cannot be created."),
			*GetFName().ToString());
		return;
	}

	if (BodyAttachment2.GetRigidBody() != nullptr && RigidBody2 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Prismatic constraint %s: could not get Rigid Body Actor from Body Attachment 2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	// Ok if second is nullptr, means that the first body is constrainted to the world.
	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2, &FrameRotation2);
}
