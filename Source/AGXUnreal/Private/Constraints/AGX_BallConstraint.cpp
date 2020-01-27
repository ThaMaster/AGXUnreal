// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_BallConstraint.h"

#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

AAGX_BallConstraint::AAGX_BallConstraint()
	: AAGX_Constraint(
		  {EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_3})
{
}

AAGX_BallConstraint::~AAGX_BallConstraint()
{
}

void AAGX_BallConstraint::CreateNativeImpl()
{
	NativeBarrier.Reset(new FBallJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (!RigidBody1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Ball constraint %s: could not get Rigid Body Actor from Body Attachment 1."),
			*GetFName().ToString());
		return;
	}

	if ((BodyAttachment2.RigidBodyActor && !RigidBody2))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Ball constraint %s: could not get Rigid Body Actor from Body Attachment 2."),
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
