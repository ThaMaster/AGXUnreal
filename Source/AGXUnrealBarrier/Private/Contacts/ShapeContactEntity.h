#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

struct FShapeContactEntity
{
	agxCollide::GeometryContact Native;

	FShapeContactEntity() = default;
	FShapeContactEntity(agxCollide::GeometryContact InNative)
		: Native(InNative)
	{
	}
};
