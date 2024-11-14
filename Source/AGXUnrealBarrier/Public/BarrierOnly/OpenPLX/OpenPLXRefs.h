#pragma once

// OpenPLX includes.
#include "Brick/brickagx/InputSignalListener.h"
#include "Brick/brickagx/OutputSignalListener.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/ref_ptr.h>
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <vector>

struct FInputSignalHandlerRef
{
	agx::ref_ptr<BrickAgx::InputSignalListener> Native;

	FInputSignalHandlerRef() = default;
	FInputSignalHandlerRef(agxSDK::Assembly* Assembly)
		: Native(new BrickAgx::InputSignalListener(Assembly))
	{
	}
};

struct FOutputSignalHandlerRef
{
	agx::ref_ptr<BrickAgx::OutputSignalListener> Native;

	FOutputSignalHandlerRef() = default;
	FOutputSignalHandlerRef(
		agxSDK::Assembly* Assembly, const std::shared_ptr<Brick::Core::Object>& PlxModel)
		: Native(new BrickAgx::OutputSignalListener(Assembly, PlxModel))
	{
	}
};

struct FPLXModelDatum
{
	agx::ref_ptr<BrickAgx::InputSignalListener> InputSignalHandler;
	agxSDK::AssemblyRef Assembly;
};

struct FPLXModelData
{
	std::vector<FPLXModelDatum> ModelData;
};
