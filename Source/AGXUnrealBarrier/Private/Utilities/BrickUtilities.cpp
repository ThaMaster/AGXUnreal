// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/BrickUtilities.h"

// Brick includes.
#include "Brick/Physics3D/Physics3D_all.h"
#include <Physics3D/Signals/LinearVelocityMotorVelocityInput.h>

// Standard library includes.
#include <memory>

namespace BrickUtilities_helpers
{
	void GetInputs(Brick::Physics3D::System* System, TArray<FPLX_Input>& OutInputs)
	{
		if (System == nullptr)
			return;

		for (auto& Subsystem : System->getValues<Brick::Physics3D::System>())
		{
			GetInputs(System, OutInputs);
		}

		for (auto& Input : System->getValues<Brick::Physics::Signals::Input>())
		{
			if (auto LvmvI = std::dynamic_pointer_cast<
					Brick::Physics3D::Signals::LinearVelocityMotorVelocityInput>(Input))
			{
				OutInputs.Add(FPLX_LinearVelocityMotorVelocityInput(Convert(LvmvI->motor()->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Input: %s"), *Convert(Input->getName()));
		}
	}
}

TArray<FPLX_Input> FBrickUtilities::GetInputs(Brick::Physics3D::System* System)
{
	TArray<FPLX_Input> Inputs;
	BrickUtilities_helpers::GetInputs(System, Inputs);
	return Inputs;
}
