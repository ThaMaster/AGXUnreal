// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Brick/AGX_BrickInputs.h"

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
	static TArray<FPLX_Input> GetInputs(Brick::Physics3D::System* System);
};
