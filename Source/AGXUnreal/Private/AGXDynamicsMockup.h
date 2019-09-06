#pragma once

#include "Logging/LogMacros.h"

namespace agx
{
	using agxCollide_Box = void;
	using agxCollide_BoxRef = void*;

	using agxCollide_Geometry = void;
	using agxCollide_GeometryRef = void*;

	using agx_RigidBody = void;
	using agx_RigidBodyRef = void*;

	inline void* allocate(const TCHAR* classname)
	{
		UE_LOG(LogTemp, Log, TEXT("AGX_CALL: Allocate %s"), classname);
		static intptr_t next = 0;
		++next;
		return reinterpret_cast<void*>(next);
	}

	inline void call(const TCHAR* call)
	{
		UE_LOG(LogTemp, Log, TEXT("AGX_CALL: %s"), call);
	}
}
