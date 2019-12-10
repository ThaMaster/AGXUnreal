#include "Constraints/ConstraintBarrier.h"

#include "AGXRefs.h"
#include "AGXBarrierFactories.h"
#include "TypeConversions.h"

#include <Misc/AssertionMacros.h>

FConstraintBarrier::FConstraintBarrier()
	: NativeRef {new FConstraintRef}
{
}

FConstraintBarrier::FConstraintBarrier(std::unique_ptr<FConstraintRef> Native)
	: NativeRef(std::move(Native))
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
	const FRigidBodyBarrier* RigidBody1, const FVector* FramePosition1, const FQuat* FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector* FramePosition2, const FQuat* FrameRotation2)
{
	check(!HasNative());

	AllocateNativeImpl(RigidBody1, FramePosition1, FrameRotation1, RigidBody2, FramePosition2, FrameRotation2);
}

void FConstraintBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FConstraintBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FConstraintBarrier::GetName() const
{
	check(HasNative());
	agx::String NameAGX = NativeRef->Native->getName();
	FString NameUnreal = Convert(NameAGX);
	return NameUnreal;
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

bool FConstraintBarrier::HasFirstBody() const
{
	check(HasNative());
	return NativeRef->Native->getNumBodies() >= 1;
}

bool FConstraintBarrier::HasSecondBody() const
{
	check(HasNative());
	return NativeRef->Native->getNumBodies() >= 2;
}

namespace
{
	FRigidBodyBarrier GetBodyAsBarrier(agx::Constraint* Constraint, agx::UInt Index)
	{
		if (Index >= Constraint->getNumBodies())
		{
			return FRigidBodyBarrier();
		}
		return CreateRigidBodyBarrier(Constraint->getBodyAt(Index));
	}
}

FRigidBodyBarrier FConstraintBarrier::GetFirstBody() const
{
	check(HasNative());
	return GetBodyAsBarrier(NativeRef->Native, agx::UInt(0));
}

FRigidBodyBarrier FConstraintBarrier::GetSecondBody() const
{
	check(HasNative());
	return GetBodyAsBarrier(NativeRef->Native, agx::UInt(1));
}

namespace
{
	agx::Frame* GetFrame(const agx::Constraint& Native, int32 IndexUnreal)
	{
		check(IndexUnreal >= 0 && IndexUnreal < 2);
		agx::UInt IndexAGX = static_cast<agx::UInt>(IndexUnreal);
		return Native.getAttachment(IndexAGX)->getFrame();
	}
}

FVector FConstraintBarrier::GetLocalLocation(int32 Index) const
{
	check(HasNative());
	agx::Vec3 TranslateAGX = GetFrame(*NativeRef->Native, Index)->getLocalTranslate();
	FVector TranslateUnreal = ConvertVector(TranslateAGX);
	return TranslateUnreal;
}

FRotator FConstraintBarrier::GetLocalRotation(int32 Index) const
{
	check(HasNative());
	agx::Quat RotateAGX = GetFrame(*NativeRef->Native, Index)->getLocalRotate();
	FQuat RotateUnreal = Convert(RotateAGX);
	return FRotator(RotateUnreal);
}
