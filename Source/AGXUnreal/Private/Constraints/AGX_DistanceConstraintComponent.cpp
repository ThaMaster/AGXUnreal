#include "Constraints/AGX_DistanceConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/DistanceJointBarrier.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

UAGX_DistanceConstraintComponent::UAGX_DistanceConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  TArray<EDofFlag> {
			  // All common DOFs are free.
		  },
		  /*bIsSecondaryConstraintRotational*/ false,
		  /*bIsLockControllerEditable*/ false) // disable because the native impl uses Lock
											   // Controller implicitly!
{
	/// \todo Determine if this is needed, or if the FAGX_ConstraintFrameComponent constructor
	/// does what we want.
	BodyAttachment1.FrameDefiningComponent.Clear();
	BodyAttachment2.FrameDefiningComponent.Clear();
}

UAGX_DistanceConstraintComponent::~UAGX_DistanceConstraintComponent()
{
}

void UAGX_DistanceConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FDistanceJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Distance constraint %s: could not get Rigid Body Actor from Body Attachment 1. "
				 "Constraint cannot be created."),
			*GetFName().ToString());
		return;
	}

	if (BodyAttachment2.GetRigidBody() != nullptr && RigidBody2 == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Distance constraint %s: could not get Rigid Body Actor from Body Attachment 2."),
			*GetFName().ToString());
		return;
	}

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	// Ok if second is nullptr, means that the first body is constrainted to the world.
	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2,
		&FrameRotation2);
}
