// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/PLX_Inputs.h"

namespace Brick
{
	namespace Physics3D
	{
		class System;
	}
}

class FBrickUtilities
{
public:
	static TArray<TUniquePtr<FPLX_Input>> GetInputs(Brick::Physics3D::System* System);
};
