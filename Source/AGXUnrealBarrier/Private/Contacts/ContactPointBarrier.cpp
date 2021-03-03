#include "Contacts/ContactPointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointEntity.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

FContactPointBarrier::FContactPointBarrier()
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
	NativeEntity->Native = InOther.NativeEntity->Native;
	return *this;
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
