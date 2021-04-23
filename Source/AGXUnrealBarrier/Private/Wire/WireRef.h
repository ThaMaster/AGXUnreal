#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

struct FWireRef
{
	agxWire::WireRef Native;

	FWireRef() = default;
	FWireRef(agxWire::Wire* InNative)
		: Native(InNative)
	{
	}
};
