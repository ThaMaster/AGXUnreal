// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/ElementaryConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agx/ElementaryConstraint.h"
#include "EndAGXIncludes.h"

FElementaryConstraintBarrier::FElementaryConstraintBarrier()
	: NativeRef(new FElementaryConstraintRef())
{
}

FElementaryConstraintBarrier::FElementaryConstraintBarrier(
	std::unique_ptr<FElementaryConstraintRef> InNative)
	: NativeRef(std::move(InNative))
{
}

FElementaryConstraintBarrier::~FElementaryConstraintBarrier()
{
	// Must have a non-inlined destructor because the NativeRef destructor must be able to see the
	// full definition of the pointed-to type, and we are not allowed to include F*Ref / F*Ptr
	// types in the header file.
}

FElementaryConstraintBarrier& FElementaryConstraintBarrier::operator=(const FElementaryConstraintBarrier& Other)
{
	NativeRef->Native = Other.NativeRef->Native;
	return *this;
}

bool FElementaryConstraintBarrier::HasNative() const
{
	return NativeRef.get() != nullptr && NativeRef->Native.get() != nullptr;
}

void FElementaryConstraintBarrier::SetNative(FElementaryConstraintRef* InNativeRef)
{
	if (InNativeRef == nullptr)
	{
		NativeRef->Native = nullptr;
		return;
	}
	NativeRef->Native = InNativeRef->Native;
}

FElementaryConstraintRef* FElementaryConstraintBarrier::GetNative()
{
	return NativeRef.get();
}

const FElementaryConstraintRef* FElementaryConstraintBarrier::GetNative() const
{
	return NativeRef.get();
}

void FElementaryConstraintBarrier::SetEnable(bool bEnabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(bEnabled);
}

bool FElementaryConstraintBarrier::GetEnable() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

bool FElementaryConstraintBarrier::IsActive() const
{
	check(HasNative());
	return NativeRef->Native->isActive();
}

void FElementaryConstraintBarrier::SetCompliance(double InCompliance, int32 InRow)
{
	check(HasNative());
	NativeRef->Native->setCompliance(InCompliance, InRow);
}

void FElementaryConstraintBarrier::SetCompliance(double InCompliance)
{
	check(HasNative());
	NativeRef->Native->setCompliance(InCompliance);
}

double FElementaryConstraintBarrier::GetCompliance(int32 InRow) const
{
	check(HasNative());
	return NativeRef->Native->getCompliance(InRow);
}

void FElementaryConstraintBarrier::setElasticity(double Elasticity, int Row)
{
	check(HasNative());
	NativeRef->Native->setElasticity(Elasticity, Row);
}

void FElementaryConstraintBarrier::setElasticity(double Elasticity)
{
	check(HasNative());
	NativeRef->Native->setElasticity(Elasticity);
}

double FElementaryConstraintBarrier::getElasticity(int32 Row) const
{
	check(HasNative());
	return NativeRef->Native->getElasticity(Row);
}

void FElementaryConstraintBarrier::SetSpookDamping(double InDamping, int32 InRow)
{
	check(HasNative());
	NativeRef->Native->setDamping(InDamping, InRow);
}

void FElementaryConstraintBarrier::SetSpookDamping(double InDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(InDamping);
}

double FElementaryConstraintBarrier::GetSpookDamping(int32 InRow) const
{
	check(HasNative());
	return NativeRef->Native->getDamping(InRow);
}

void FElementaryConstraintBarrier::SetForceRange(double InMin, double InMax, int32 InRow)
{
	SetForceRange(FAGX_RealInterval(InMin, InMax), InRow);
}

void FElementaryConstraintBarrier::SetForceRange(FAGX_RealInterval InForceRange, int32 InRow)
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = Convert(InForceRange);
	NativeRef->Native->setForceRange(ForceRangeAGX, InRow);
}

FDoubleInterval FElementaryConstraintBarrier::GetForceRange(int32 InRow) const
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = NativeRef->Native->getForceRange(InRow);
	const FDoubleInterval ForceRange = Convert(ForceRangeAGX);
	return ForceRange;
}

double FElementaryConstraintBarrier::GetForceRangeMin(int32 InRow) const
{
	check(HasNative());
	return NativeRef->Native->getForceRange(InRow).lower();
}

double FElementaryConstraintBarrier::GetForceRangeMax(int32 InRow) const
{
	check(HasNative());
	return NativeRef->Native->getForceRange(InRow).upper();
}

double FElementaryConstraintBarrier::GetForce(int32 InRow) const
{
	check(HasNative());
	return NativeRef->Native->getCurrentForce(InRow);
}

int32 FElementaryConstraintBarrier::GetNumRows() const
{
	check(HasNative());
	return NativeRef->Native->getNumRows();
}

FString FElementaryConstraintBarrier::GetName() const
{
	check(HasNative());
	agx::Name NameAGX = NativeRef->Native->getName();
	FString Name = Convert(NameAGX);
	return Name;
}
