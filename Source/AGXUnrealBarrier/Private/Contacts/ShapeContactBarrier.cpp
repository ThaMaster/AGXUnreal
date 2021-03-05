#include "Contacts/ShapeContactBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Contacts/ShapeContactEntity.h"
#include "AGXBarrierFactories.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Contacts.h>
#include "EndAGXIncludes.h"

FShapeContactBarrier::FShapeContactBarrier()
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
	NativeEntity->Native = InOther.NativeEntity->Native;
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


int32 FShapeContactBarrier::GetNumContacts() const
{
	check(HasNative());
	const size_t NumPointsAGX = NativeEntity->Native.points().size();
	const int32 MaxAllowed = std::numeric_limits<int32>::max() - 1;
	if (NumPointsAGX > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("A Shape Contact has too many contact points, %zu truncated to %d."),
			NumPointsAGX, MaxAllowed);
		return MaxAllowed;
	}
	else
	{
		return static_cast<int32>(NumPointsAGX);
	}
}

TArray<FContactPointBarrier> FShapeContactBarrier::GetContactPoints() const
{
	check(HasNative());

	// Read contact points from AGX Dynamics
	agxData::Array<agxCollide::ContactPoint>& ContactPointsAGX = NativeEntity->Native.points();
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
		agxCollide::ContactPoint ContactPointAGX = ContactPointsAGX[I];

		// We're pessimizing the MaxAllowed limit since any invalid Contact Point will still count
		// towards the limit. I don't think this will ever matter, but if it does simply don't
		// truncate NumContacts, let I loop over all Geometry Contacts, and break when the TArray is
		// full.
		if (!ContactPointAGX.isValid())
		{
			continue;
		}

		ContactPoints.Add(AGXBarrierFactories::CreateContactPointBarrier(ContactPointAGX));
	}

	return ContactPoints;
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
