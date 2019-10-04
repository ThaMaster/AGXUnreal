// Fill out your copyright notice in the Description page of Project Settings.


#include "LockJointBarrier.h"

#include "AGXRefs.h"
#include "NativeConversions.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"


FLockJointBarrier::FLockJointBarrier()
	: FConstraintBarrier()
{
}

FLockJointBarrier::~FLockJointBarrier()
{
}

void FLockJointBarrier::AllocateNativeImpl(const FRigidBodyBarrier *Rb1, const FRigidBodyBarrier *Rb2)
{
	check(!HasNative());

	NativeRef->Native = new agx::LockJoint(
		GetNativeFromBarrier<agx::RigidBody>(Rb1),
		GetNativeFromBarrier<agx::RigidBody>(Rb2)); // TODO: Pass in frames as well!
}
