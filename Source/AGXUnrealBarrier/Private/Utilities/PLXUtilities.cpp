// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "TypeConversions.h"

// OpenPLX includes.
#include "OpenPLX/openplx/OpenPlxContext.h"
#include "OpenPLX/openplx/OpenPlxContextInternal.h"
#include "OpenPLX/openplx/OpenPlxCoreAPI.h"
#include "OpenPLX/agx-openplx/AgxOpenPlxApi.h"
#include "OpenPLX/Math/Math_all.h"
#include "OpenPLX/Physics/Physics_all.h"
#include "OpenPLX/Physics1D/Physics1D_all.h"
#include "OpenPLX/Physics3D/Physics3D_all.h"
#include "OpenPLX/Physics/Signals/LinearVelocity1DInput.h"
#include "OpenPLX/DriveTrain/DriveTrain_all.h"
#include "OpenPLX/Robotics/Robotics_all.h"
#include "OpenPLX/Simulation/Simulation_all.h"
#include "OpenPLX/Vehicles/Vehicles_all.h"
#include "OpenPLX/Terrain/Terrain_all.h"
#include "OpenPLX/Visuals/Visuals_all.h"
#include "OpenPLX/Urdf/Urdf_all.h"


// Unreal Engine includes.
#include "Templates/UniquePtr.h"

// Standard library includes.
#include <memory>

namespace PLXUtilities_helpers
{
	void GetInputs(openplx::Physics3D::System* System, TArray<TUniquePtr<FPLX_Input>>& OutInputs)
	{
		if (System == nullptr)
			return;

		for (auto& Subsystem : System->getValues<openplx::Physics3D::System>())
		{
			GetInputs(System, OutInputs);
		}

		for (auto& Input : System->getValues<openplx::Physics::Signals::Input>())
		{
			if (auto Lvmvi = std::dynamic_pointer_cast<openplx::Physics::Signals::LinearVelocity1DInput>(
						Input))
			{
				OutInputs.Emplace(MakeUnique<FPLX_LinearVelocity1DInput>(
					Convert(Input->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Input: %s"), *Convert(Input->getName()));
		}
	}

	void GetOutputs(openplx::Physics3D::System* System, TArray<TUniquePtr<FPLX_Output>>& OutOutputs)
	{
		if (System == nullptr)
			return;

		for (auto& Subsystem : System->getValues<openplx::Physics3D::System>())
		{
			GetOutputs(System, OutOutputs);
		}

		for (auto& Output : System->getValues<openplx::Physics::Signals::Output>())
		{
			if (auto Hao =
					std::dynamic_pointer_cast<openplx::Physics::Signals::AngleOutput>(Output))
			{
				OutOutputs.Emplace(
					MakeUnique<FPLX_AngleOutput>(Convert(Output->getName())));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Output: %s"), *Convert(Output->getName()));
		}
	}

	std::shared_ptr<openplx::Core::Api::OpenPlxContext> CreatePLXContext(
		std::shared_ptr<agxopenplx::AgxCache> AGXCache)
	{
		const FString PLXBundlesPath = FPaths::Combine(
			FAGX_Environment::GetPluginSourcePath(), "Thirdparty", "agx", "openplxbundles");
		auto PLXCtx = std::make_shared<openplx::Core::Api::OpenPlxContext>(
			std::vector<std::string>({Convert(PLXBundlesPath)}));

		auto InternalContext = openplx::Core::Api::OpenPlxContextInternal::fromContext(*PLXCtx);
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

		agxopenplx::register_plugins(*PLXCtx, AGXCache);
		return PLXCtx;
	}

	openplx::Core::ObjectPtr LoadModelFromFile(
		const std::string& OpenPLXFile, std::shared_ptr<agxopenplx::AgxCache> AGXCache)
	{
		auto Context = CreatePLXContext(AGXCache);
		if (Context == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Error Creating OpenPLX Context"));
			return nullptr;
		}

		auto LoadedModel = openplx::Core::Api::loadModelFromFile(OpenPLXFile, {}, *Context);

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

openplx::Core::ObjectPtr FPLXUtilities::LoadModel(
	const FString& Filename, std::shared_ptr<agxopenplx::AgxCache> AGXCache)
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

TArray<TUniquePtr<FPLX_Input>> FPLXUtilities::GetInputs(openplx::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Input>> Inputs;
	if (System == nullptr)
		return Inputs;

	PLXUtilities_helpers::GetInputs(System, Inputs);
	return Inputs;
}

TArray<TUniquePtr<FPLX_Output>> FPLXUtilities::GetOutputs(openplx::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Output>> Outputs;
	if (System == nullptr)
		return Outputs;

	PLXUtilities_helpers::GetOutputs(System, Outputs);
	return Outputs;
}
