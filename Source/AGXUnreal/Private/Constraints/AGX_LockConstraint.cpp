// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_LockConstraint.h"

#include "Constraints/ConstraintBarrier.h"
#include "Constraints/LockJointBarrier.h"

class FRigidBodyBarrier;

AAGX_LockConstraint::AAGX_LockConstraint()
	: AAGX_Constraint({EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
					   EDofFlag::DOF_FLAG_TRANSLATIONAL_3, EDofFlag::DOF_FLAG_ROTATIONAL_1,
					   EDofFlag::DOF_FLAG_ROTATIONAL_2, EDofFlag::DOF_FLAG_ROTATIONAL_3})
{
}

AAGX_LockConstraint::~AAGX_LockConstraint()
{
}

void AAGX_LockConstraint::CreateNativeImpl()
{
	NativeBarrier.Reset(new FLockJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	// TODO: Change checks below to more gentle user errors instead of crashes!
	check(RigidBody1); // Must be set!
	check(!(BodyAttachment2.RigidBodyActor && !RigidBody2)); // Actor has no Rigid Body!

	FVector FrameLocation1 = BodyAttachment1.GetLocalFrameLocation();
	FVector FrameLocation2 = BodyAttachment2.GetLocalFrameLocation();

	FQuat FrameRotation1 = BodyAttachment1.GetLocalFrameRotation();
	FQuat FrameRotation2 = BodyAttachment2.GetLocalFrameRotation();

	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2,
		&FrameRotation2); // ok if second is nullptr
}
