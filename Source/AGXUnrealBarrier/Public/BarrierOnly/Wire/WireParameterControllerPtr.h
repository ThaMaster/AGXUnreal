// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Wire.h>
#include <agxWire/WireParameterController.h>
#include "EndAGXIncludes.h"

struct FWireParameterControllerPtr
{
	// The AGX Dynamics Wire that owns the WireParameterController
	agxWire::WireObserver NativeWire {nullptr};

	FWireParameterControllerPtr() = default;
	FWireParameterControllerPtr(agxWire::Wire* InNative)
		: NativeWire(InNative)
	{
	}
};
