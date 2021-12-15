// Copyright 2021, Algoryx Simulation AB.


#include "Contacts/ContactPointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointEntity.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

FContactPointBarrier::FContactPointBarrier()
	: NativeEntity {new FContactPointEntity}
{
}

FContactPointBarrier::FContactPointBarrier(std::unique_ptr<FContactPointEntity> InNativeEntity)
	: NativeEntity(std::move(InNativeEntity))
{
}

FContactPointBarrier::FContactPointBarrier(FContactPointBarrier&& InOther)
	: NativeEntity(std::move(InOther.NativeEntity))
{
}

FContactPointBarrier::FContactPointBarrier(const FContactPointBarrier& InOther)
	: NativeEntity(new FContactPointEntity(InOther.NativeEntity->Native))
{
}

FContactPointBarrier::~FContactPointBarrier()
{
}

FContactPointBarrier& FContactPointBarrier::operator=(const FContactPointBarrier& InOther)
{
	if (InOther.HasNative())
	{
		NativeEntity->Native = InOther.NativeEntity->Native;
	}
	else
	{
		NativeEntity->Native = agxCollide::ContactPoint();
	}
	return *this;
}

float FContactPointBarrier::GetDepth() const
{
	check(HasNative());
	return ConvertDistance(NativeEntity->Native.depth());
}

FVector FContactPointBarrier::GetLocation() const
{
	check(HasNative());
	return ConvertDisplacement(NativeEntity->Native.point());
}

FVector FContactPointBarrier::GetNormal() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.normal());
}

FVector FContactPointBarrier::GetTangentU() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.tangentU());
}

FVector FContactPointBarrier::GetTangentV() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.tangentV());
}

FVector FContactPointBarrier::GetForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getForce());
}

FVector FContactPointBarrier::GetNormalForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getNormalForce());
}

FVector FContactPointBarrier::GetTangentialForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getTangentialForce());
}

FVector FContactPointBarrier::GetLocalForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.localForce());
}

FVector FContactPointBarrier::GetWitnessPoint(int32 Index) const
{
	check(HasNative());
	return ConvertDisplacement(NativeEntity->Native.getWitnessPoint(Index));
}

float FContactPointBarrier::GetArea() const
{
	check(HasNative());
	return ConvertDistance2(NativeEntity->Native.area());
}

bool FContactPointBarrier::IsEnabled() const
{
	check(HasNative());
	return NativeEntity->Native.enabled();
}

bool FContactPointBarrier::HasNative() const
{
	return NativeEntity.get() != nullptr && NativeEntity->Native.isValid();
}

FContactPointEntity* FContactPointBarrier::GetNative()
{
	return NativeEntity.get();
}

const FContactPointEntity* FContactPointBarrier::GetNative() const
{
	return NativeEntity.get();
}
