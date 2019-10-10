// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_PrismaticConstraint.h"

#include "Constraints/PrismaticBarrier.h"

class FRigidBodyBarrier;


AAGX_PrismaticConstraint::AAGX_PrismaticConstraint()
	: AAGX_Constraint1DOF(
		{
			EDofFlag::DOF_FLAG_ROTATIONAL_1,
			EDofFlag::DOF_FLAG_ROTATIONAL_2,
			EDofFlag::DOF_FLAG_ROTATIONAL_3,
			EDofFlag::DOF_FLAG_TRANSLATIONAL_1,
			EDofFlag::DOF_FLAG_TRANSLATIONAL_2
		})
{
}


AAGX_PrismaticConstraint::~AAGX_PrismaticConstraint()
{

}


void AAGX_PrismaticConstraint::CreateNativeImpl()
{
	NativeBarrier.Reset(new FPrismaticBarrier());

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
		RigidBody1, &FrameLocation1, &FrameRotation1,
		RigidBody2, &FrameLocation2, &FrameRotation2,  // ok if second is nullptr
		GetWorld());
}
