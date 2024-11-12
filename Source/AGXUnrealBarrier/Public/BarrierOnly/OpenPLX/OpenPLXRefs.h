#pragma once

// OpenPLX includes.
#include "Brick/brickagx/InputSignalListener.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/ref_ptr.h>
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>

struct FInputSignalHandlerRef
{
	agx::ref_ptr<BrickAgx::InputSignalListener> Native;

	FInputSignalHandlerRef() = default;
	FInputSignalHandlerRef(agxSDK::Assembly* Assembly)
		: Native(new BrickAgx::InputSignalListener(Assembly))
	{
	}
};
