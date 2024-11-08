// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"

// OpenPLX includes.
#include "Brick/Physics3D/Physics3D_all.h"
#include "Physics3D/Signals/LinearVelocityMotorVelocityInput.h"

// Unreal Engine includes.
#include "Templates/UniquePtr.h"

// Standard library includes.
#include <memory>

namespace PLXUtilities_helpers
{
	void GetInputs(Brick::Physics3D::System* System, TArray<TUniquePtr<FPLX_Input>>& OutInputs)
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
				OutInputs.Emplace(MakeUnique<FPLX_LinearVelocityMotorVelocityInput>(
					Convert(LvmvI->motor()->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Input: %s"), *Convert(Input->getName()));
		}
	}
}

TArray<TUniquePtr<FPLX_Input>> FBrickUtilities::GetInputs(Brick::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Input>> Inputs;
	PLXUtilities_helpers::GetInputs(System, Inputs);
	return Inputs;
}
