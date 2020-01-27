// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_CylindricalConstraint.h"

#include "Constraints/CylindricalJointBarrier.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

AAGX_CylindricalConstraint::AAGX_CylindricalConstraint()
	: AAGX_Constraint2DOF(
		  {EDofFlag::DOF_FLAG_ROTATIONAL_1, EDofFlag::DOF_FLAG_ROTATIONAL_2,
		   EDofFlag::DOF_FLAG_TRANSLATIONAL_1, EDofFlag::DOF_FLAG_TRANSLATIONAL_2},
		  /*bIsSecondaryConstraint1Rotational*/ false,
		  /*bIsSecondaryConstraint2Rotational*/ true)
{
}

AAGX_CylindricalConstraint::~AAGX_CylindricalConstraint()
{
}

void AAGX_CylindricalConstraint::CreateNativeImpl()
{
	NativeBarrier.Reset(new FCylindricalJointBarrier());

	FRigidBodyBarrier* RigidBody1 = BodyAttachment1.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);
	FRigidBodyBarrier* RigidBody2 = BodyAttachment2.GetRigidBodyBarrier(/*CreateIfNeeded*/ true);

	if (!RigidBody1)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Cylindrical constraint %s: could not get Rigid Body Actor from Body Attachment 1."),
			*GetFName().ToString());
		return;
	}

	if ((BodyAttachment2.RigidBodyActor && !RigidBody2))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Cylindrical constraint %s: could not get Rigid Body Actor from Body Attachment 2."),
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
