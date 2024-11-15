#pragma once

// OpenPLX includes.
#include "Brick/brickagx/AgxCache.h"
#include "Brick/brickagx/Signals.h"
#include "Brick/brickagx/InputSignalListener.h"
#include "Brick/brickagx/OutputSignalListener.h"
#include "Brick/Physics3D/System.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/ref_ptr.h>
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <vector>
#include <unordered_map>

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
	std::shared_ptr<BrickAgx::AgxCache> AGXCache;
	Brick::Core::ObjectPtr PLXModel;
	agx::ref_ptr<BrickAgx::InputSignalListener> InputSignalListener;
	std::unordered_map<std::string, agx::ref_ptr<BrickAgx::OutputSignalListener>> OutputSignalListeners;
	agxSDK::AssemblyRef Assembly;
	std::unordered_map<std::string, std::shared_ptr<Brick::Physics::Signals::Input>> Inputs;
};

struct FPLXModelData
{
	std::vector<FPLXModelDatum> ModelData;
};
