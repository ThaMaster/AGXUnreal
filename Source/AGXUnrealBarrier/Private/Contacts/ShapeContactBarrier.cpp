// Copyright 2024, Algoryx Simulation AB.

#include "Contacts/ShapeContactBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXBarrierFactories.h"
#include "Contacts/ShapeContactEntity.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Contacts.h>
#include "EndAGXIncludes.h"

FShapeContactBarrier::FShapeContactBarrier()
	: NativeEntity {new FShapeContactEntity}
{
}

FShapeContactBarrier::FShapeContactBarrier(std::unique_ptr<FShapeContactEntity> NativeEntity)
	: NativeEntity(std::move(NativeEntity))
{
}

FShapeContactBarrier::FShapeContactBarrier(FShapeContactBarrier&& Other)
	: NativeEntity(std::move(Other.NativeEntity))
{
}

FShapeContactBarrier::~FShapeContactBarrier()
{
}

FShapeContactBarrier& FShapeContactBarrier::operator=(const FShapeContactBarrier& InOther)
{
	if (InOther.HasNative())
	{
		NativeEntity->Native = InOther.NativeEntity->Native;
	}
	else
	{
		NativeEntity->Native = agxCollide::GeometryContact();
	}
	return *this;
}

bool FShapeContactBarrier::IsEnabled() const
{
	check(HasNative());
	return NativeEntity->Native.isEnabled();
}

EAGX_ContactState FShapeContactBarrier::GetContactState()
{
	check(HasNative());
	return EAGX_ContactState(NativeEntity->Native.state());
}

FRigidBodyBarrier FShapeContactBarrier::GetBody1() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeEntity->Native.rigidBody(0);
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

FRigidBodyBarrier FShapeContactBarrier::GetBody2() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeEntity->Native.rigidBody(1);
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

FEmptyShapeBarrier FShapeContactBarrier::GetShape1() const
{
	check(HasNative());
	agxCollide::Geometry* Geometry = NativeEntity->Native.geometry(0);
	return AGXBarrierFactories::CreateEmptyShapeBarrier(Geometry);
}

FEmptyShapeBarrier FShapeContactBarrier::GetShape2() const
{
	check(HasNative());
	agxCollide::Geometry* Geometry = NativeEntity->Native.geometry(1);
	return AGXBarrierFactories::CreateEmptyShapeBarrier(Geometry);
}

int32 FShapeContactBarrier::GetNumContactPoints() const
{
	check(HasNative());
	const size_t NumPointsAGX = NativeEntity->Native.points().size();
	const int32 MaxAllowed = std::numeric_limits<int32>::max() - 1;
	if (NumPointsAGX > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("A Shape Contact has too many contact points, %zu truncated to %d."), NumPointsAGX,
			MaxAllowed);
		return MaxAllowed;
	}
	else
	{
		return static_cast<int32>(NumPointsAGX);
	}
}

FVector FShapeContactBarrier::CalculateRelativeVelocity(int32 PointIndex) const
{
	check(HasNative());
	if (PointIndex < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Negative index %d passed to FShapeContactBarrier::CalculateRelativeVelocity."),
			PointIndex);
		return FVector::ZeroVector;
	}
	const size_t PointIndexAGX = static_cast<size_t>(PointIndex);
	const size_t NumPoints = NativeEntity->Native.points().size();
	if (PointIndexAGX >= NumPoints)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large index passed to FShapeContactBarrier::CalculateRelativeVelocity. Given "
				 "%zu but only have %zu contact points."),
			PointIndexAGX, NumPoints);
	}
	return ConvertDisplacement(NativeEntity->Native.calculateRelativeVelocity(PointIndexAGX));
}

TArray<FContactPointBarrier> FShapeContactBarrier::GetContactPoints() const
{
	check(HasNative());

	// Read contact points from AGX Dynamics
	agxCollide::ContactPointVector& ContactPointsAGX = NativeEntity->Native.points();
	size_t NumContactPoints = ContactPointsAGX.size();
	// Save one for INVALID_INDEX/InvalidIndex.
	const int32 MaxAllowed = std::numeric_limits<int32>::max() - 1;
	if (NumContactPoints > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Too many ContactsPoints, %zu, will only see the first %d."),
			NumContactPoints, MaxAllowed);
		NumContactPoints = MaxAllowed;
	}

	// Wrap each Contact Point in a Barrier.
	TArray<FContactPointBarrier> ContactPoints;
	ContactPoints.Reserve(NumContactPoints);
	for (int32 I = 0; I < NumContactPoints; ++I)
	{
		// We add the contact point even if it is disabled or invalid because we want the contact
		// point indices exposed to Unreal Engine to match the AGX Dynamics indices. Because there
		// are index-based contact point accessor and helper functions that take a contact point
		// index and it is important that everyone agrees on each contact point's index.
		ContactPoints.Add(AGXBarrierFactories::CreateContactPointBarrier(ContactPointsAGX[I]));
	}

	return ContactPoints;
}

FContactPointBarrier FShapeContactBarrier::GetContactPoint(int32 Index) const
{
	check(HasNative());

	if (Index < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Negative index passed to FShapeContactBarrier::GetContactPoint."));
		return FContactPointBarrier();
	}

	const size_t IndexAGX = static_cast<size_t>(Index);
	agxCollide::ContactPointVector& Points = NativeEntity->Native.points();
	if (IndexAGX >= Points.size())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large index passed to FShapeContactBarrier::GetContactPoint. Given %zu but "
				 "only have %zu contact points."),
			IndexAGX, Points.size());
		return FContactPointBarrier();
	}

	return AGXBarrierFactories::CreateContactPointBarrier(Points[IndexAGX]);
}

bool FShapeContactBarrier::HasNative() const
{
	return NativeEntity.get() != nullptr && NativeEntity->Native.isValid();
}

FShapeContactEntity* FShapeContactBarrier::GetNative()
{
	return NativeEntity.get();
}

const FShapeContactEntity* FShapeContactBarrier::GetNative() const
{
	return NativeEntity.get();
}
