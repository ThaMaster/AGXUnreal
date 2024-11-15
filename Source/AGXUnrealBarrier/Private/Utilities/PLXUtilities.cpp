// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "TypeConversions.h"

// OpenPLX includes.
#include "Brick/brick/BrickContext.h"
#include "Brick/brick/BrickContextInternal.h"
#include "Brick/Brick/BrickCoreApi.h"
#include "Brick/brickagx/BrickAgxApi.h"
#include "Brick/Math/Math_all.h"
#include "Brick/Physics/Physics_all.h"
#include "Brick/Physics1D/Physics1D_all.h"
#include "Brick/Physics3D/Physics3D_all.h"
#include "Brick/Physics3D/Signals/LinearVelocityMotorVelocityInput.h"
#include "Brick/DriveTrain/DriveTrain_all.h"
#include "Brick/Robotics/Robotics_all.h"
#include "Brick/Simulation/Simulation_all.h"
#include "Brick/Vehicles/Vehicles_all.h"
#include "Brick/Terrain/Terrain_all.h"
#include "Brick/Visuals/Visuals_all.h"
#include "Brick/Urdf/Urdf_all.h"


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
			if (auto Lvmvi = std::dynamic_pointer_cast<
					Brick::Physics3D::Signals::LinearVelocityMotorVelocityInput>(Input))
			{
				OutInputs.Emplace(MakeUnique<FPLX_LinearVelocityMotorVelocityInput>(
					Convert(Input->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Input: %s"), *Convert(Input->getName()));
		}
	}

	void GetOutputs(Brick::Physics3D::System* System, TArray<TUniquePtr<FPLX_Output>>& OutOutputs)
	{
		if (System == nullptr)
			return;

		for (auto& Subsystem : System->getValues<Brick::Physics3D::System>())
		{
			GetOutputs(System, OutOutputs);
		}

		for (auto& Output : System->getValues<Brick::Physics::Signals::Output>())
		{
			if (auto Hao = std::dynamic_pointer_cast<Brick::Physics3D::Signals::HingeAngleOutput>(Output))
			{
				OutOutputs.Emplace(
					MakeUnique<FPLX_HingeAngleOutput>(Convert(Output->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Output: %s"), *Convert(Output->getName()));
		}
	}

	std::shared_ptr<Brick::Core::Api::BrickContext> CreatePLXContext(
		std::shared_ptr<BrickAgx::AgxCache> AGXCache)
	{
		const FString PLXBundlesPath = FPaths::Combine(
			FAGX_Environment::GetPluginSourcePath(), "Thirdparty", "agx", "brickbundles");
		auto PLXCtx = std::make_shared<Brick::Core::Api::BrickContext>(
			std::vector<std::string>({Convert(PLXBundlesPath)}));

		auto InternalContext = Brick::Core::Api::BrickContextInternal::fromContext(*PLXCtx);
		auto EvalCtx = InternalContext->evaluatorContext().get();

		Math_register_factories(EvalCtx);
		Physics_register_factories(EvalCtx);
		Physics1D_register_factories(EvalCtx);
		Physics3D_register_factories(EvalCtx);
		DriveTrain_register_factories(EvalCtx);
		Robotics_register_factories(EvalCtx);
		Simulation_register_factories(EvalCtx);
		Vehicles_register_factories(EvalCtx);
		Terrain_register_factories(EvalCtx);
		Visuals_register_factories(EvalCtx);
		Urdf_register_factories(EvalCtx);

		BrickAgx::register_plugins(*PLXCtx, AGXCache);
		return PLXCtx;
	}

	Brick::Core::ObjectPtr LoadModelFromFile(
		const std::string& OpenPLXFile, std::shared_ptr<BrickAgx::AgxCache> AGXCache)
	{
		auto Context = CreatePLXContext(AGXCache);
		if (Context == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Error Creating OpenPLX Context"));
			return nullptr;
		}

		auto LoadedModel = Brick::Core::Api::loadModelFromFile(OpenPLXFile, {}, *Context);

		if (Context->hasErrors())
		{
			LoadedModel = nullptr;
			for (auto Error : Context->getErrors())
				UE_LOG(LogAGX, Error, TEXT("Error in OpenPLX Context : %d"), Error->getErrorCode());

			return nullptr;
		}

		return LoadedModel;
	}
}

Brick::Core::ObjectPtr FPLXUtilities::LoadModel(
	const FString& Filename, std::shared_ptr<BrickAgx::AgxCache> AGXCache)
{
	if (!FPaths::FileExists(Filename))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not read OpenPLX file '%s'. The file does not exist."),
			*Filename);
		return nullptr;
	}

	return PLXUtilities_helpers::LoadModelFromFile(Convert(Filename), AGXCache);
}

TArray<TUniquePtr<FPLX_Input>> FPLXUtilities::GetInputs(Brick::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Input>> Inputs;
	if (System == nullptr)
		return Inputs;

	PLXUtilities_helpers::GetInputs(System, Inputs);
	return Inputs;
}

TArray<TUniquePtr<FPLX_Output>> FPLXUtilities::GetOutputs(Brick::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Output>> Outputs;
	if (System == nullptr)
		return Outputs;

	PLXUtilities_helpers::GetOutputs(System, Outputs);
	return Outputs;
}
