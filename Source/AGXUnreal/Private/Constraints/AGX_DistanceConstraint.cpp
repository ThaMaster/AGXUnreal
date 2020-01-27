// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_DistanceConstraint.h"

#include "Constraints/DistanceJointBarrier.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

AAGX_DistanceConstraint::AAGX_DistanceConstraint()
	: AAGX_Constraint1DOF(
		  TArray<EDofFlag> {
			  // All common DOFs are free.
		  },
		  /*bIsSecondaryConstraintRotational*/ false,
		  /*bIsLockControllerEditable*/ false) // disable because the native impl uses Lock
											   // Controller implicitly!
{
	BodyAttachment1.FrameDefiningActor = nullptr;
	BodyAttachment2.FrameDefiningActor = nullptr;
}

AAGX_DistanceConstraint::~AAGX_DistanceConstraint()
{
}

void AAGX_DistanceConstraint::CreateNativeImpl()
{
	NativeBarrier.Reset(new FDistanceJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Distance constraint %s: could not get Rigid Body Actor from Body Attachment 1."),
			*GetFName().ToString());
		return;
	}

	if ((BodyAttachment2.RigidBodyActor && !RigidBody2))
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

	NativeBarrier->AllocateNative(
		RigidBody1, &FrameLocation1, &FrameRotation1, RigidBody2, &FrameLocation2,
		&FrameRotation2); // ok if second is nullptr
}
