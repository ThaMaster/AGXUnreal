#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

struct FContactPointEntity
{
	agxCollide::ContactPoint Native;

	FContactPointEntity() = default;
	FContactPointEntity(agxCollide::ContactPoint InNative)
		: Native(InNative)
	{
	}
};
