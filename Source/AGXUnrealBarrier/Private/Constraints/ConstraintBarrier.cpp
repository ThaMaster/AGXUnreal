#include "Constraints/ConstraintBarrier.h"

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
	const FRigidBodyBarrier *RigidBody2, const FVector *FramePosition2, const FQuat *FrameRotation2)
{
	check(!HasNative());

	AllocateNativeImpl(
		RigidBody1, FramePosition1, FrameRotation1,
		RigidBody2, FramePosition2, FrameRotation2);
}

void FConstraintBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}


void FConstraintBarrier::SetEnable(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enable);
}


bool FConstraintBarrier::GetEnable() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}


void FConstraintBarrier::SetSolveType(int32 SolveType)
{
	check(HasNative());
	NativeRef->Native->setSolveType(agx::Constraint::SolveType(SolveType));
}


int32 FConstraintBarrier::GetSolveType() const
{
	check(HasNative());
	return int32(NativeRef->Native->getSolveType());
}


void FConstraintBarrier::SetElasticity(double Elasticity, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setElasticity(Elasticity, Dof);
}

double FConstraintBarrier::GetElasticity(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getElasticity(Dof);
}

void FConstraintBarrier::SetCompliance(double Compliance, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setCompliance(Compliance, Dof);
}

double FConstraintBarrier::GetCompliance(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getCompliance(Dof);
}

void FConstraintBarrier::SetDamping(double Damping, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setDamping(Damping, Dof);
}

double FConstraintBarrier::GetDamping(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getDamping(Dof);
}


void FConstraintBarrier::SetForceRange(double Min, double Max, int32 Dof)
{
	check(HasNative());
	return NativeRef->Native->setForceRange(agx::RangeReal(Min, Max), Dof);
}


void FConstraintBarrier::GetForceRange(double* Min, double* Max, int32 Dof) const
{
	check(HasNative());
	agx::RangeReal Range = NativeRef->Native->getForceRange(Dof);

	if (Min)
		*Min = Range.lower();

	if (Max)
		*Max = Range.upper();
}
