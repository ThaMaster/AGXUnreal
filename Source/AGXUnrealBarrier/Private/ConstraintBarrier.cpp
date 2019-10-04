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

void FConstraintBarrier::AllocateNative(const FRigidBodyBarrier *Rb1, const FRigidBodyBarrier *Rb2)
{
	check(!HasNative());
	AllocateNativeImpl(Rb1, Rb2);
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