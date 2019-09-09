#pragma once

#include "AGX_LogCategory.h"


namespace agx
{
	using agxCollide_Shape = void;
	using agxCollide_ShapeRef = void*;

	using agxCollide_Box = void;
	using agxCollide_BoxRef = void*;

	using agxCollide_Geometry = void;
	using agxCollide_GeometryRef = void*;

	using agx_RigidBody = void;
	using agx_RigidBodyRef = void*;

	inline void* allocate(const TCHAR* classname)
	{
		UE_LOG(LogAGX, Log, TEXT("AGX_CALL: Allocate %s"), classname);
		static intptr_t next = 0;
		++next;
		return reinterpret_cast<void*>(next);
	}

	inline void call(const TCHAR* call)
	{
		UE_LOG(LogAGX, Log, TEXT("AGX_CALL: %s"), call);
	}
}
