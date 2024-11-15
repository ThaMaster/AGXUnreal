// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"

// OpenPLX includes.
#include "Brick/brick/Object.h"
#include "Brick/brickagx/AgxCache.h"

namespace Brick
{
	namespace Physics3D
	{
		class System;
	}
}

class FPLXUtilities
{
public:
	static Brick::Core::ObjectPtr LoadModel(
		const FString& Filename, std::shared_ptr<BrickAgx::AgxCache> AGXCache);

	static TArray<TUniquePtr<FPLX_Input>> GetInputs(Brick::Physics3D::System* System);
	static TArray<TUniquePtr<FPLX_Output>> GetOutputs(Brick::Physics3D::System* System);
};
