#include "ConstraintBarrier.h"

#include "AGXRefs.h"

#include <Misc/AssertionMacros.h>

FConstraintBarrier::FConstraintBarrier()
	: NativeRef{ new FConstraintRef }
{
}

FConstraintBarrier::~FConstraintBarrier()
{
}

bool FConstraintBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FConstraintRef* FConstraintBarrier::GetNative()
{
	return NativeRef.get();
}

const FConstraintRef* FConstraintBarrier::GetNative() const
{
	return NativeRef.get();
}

void FConstraintBarrier::AllocateNative(
	const FRigidBodyBarrier *RigidBody1, const FVector *FramePosition1, const FQuat *FrameRotation1,
	const FRigidBodyBarrier *RigidBody2, const FVector *FramePosition2, const FQuat *FrameRotation2,
	const UWorld *World)
{
	check(!HasNative());

	AllocateNativeImpl(
		RigidBody1, FramePosition1, FrameRotation1,
		RigidBody2, FramePosition2, FrameRotation2,
		World);
}

void FConstraintBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef = nullptr;
}

void FConstraintBarrier::SetCompliance(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setCompliance(Compliance);
}

double FConstraintBarrier::GetCompliance() const
{
	check(HasNative());
	return NativeRef->Native->getCompliance(0); // TODO: Support for multiple DOF!
}

void FConstraintBarrier::SetDamping(double Damping)
{
	check(HasNative());
	NativeRef->Native->setDamping(Damping);
}

double FConstraintBarrier::GetDamping() const
{
	check(HasNative());
	return NativeRef->Native->getDamping(0); // TODO: Support for multiple DOF!
}