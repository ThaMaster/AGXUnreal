// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "TypeConversions.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/OpenPlxContext.h"
#include "openplx/OpenPlxContextInternal.h"
#include "openplx/OpenPlxCoreAPI.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "Math/Math_all.h"
#include "Physics/Physics_all.h"
#include "Physics1D/Physics1D_all.h"
#include "Physics3D/Physics3D_all.h"
#include "Physics/Signals/LinearVelocity1DInput.h"
#include "DriveTrain/DriveTrain_all.h"
#include "Robotics/Robotics_all.h"
#include "Simulation/Simulation_all.h"
#include "Vehicles/Vehicles_all.h"
#include "Terrain/Terrain_all.h"
#include "Visuals/Visuals_all.h"
#include "Urdf/Urdf_all.h"
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Paths.h"
#include "Templates/UniquePtr.h"

// Standard library includes.
#include <memory>

namespace PLXUtilities_helpers
{
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
			LogAGX, Warning, TEXT("Could not read OpenPLX file '%s'. The file does not exist."),
			*Filename);
		return nullptr;
	}

	return PLXUtilities_helpers::LoadModelFromFile(Convert(Filename), AGXCache);
}

bool FPLXUtilities::HasInputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return GetNestedObjects<openplx::Physics::Signals::Input>(*System).size() > 0;
}

bool FPLXUtilities::HasOutputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return GetNestedObjects<openplx::Physics::Signals::Output>(*System).size() > 0;
}

TArray<FPLX_Input> FPLXUtilities::GetInputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Input> Inputs;
	if (System == nullptr)
		return Inputs;

	auto InputsPLX = GetNestedObjects<openplx::Physics::Signals::Input>(*System);
	Inputs.Reserve(InputsPLX.size());
	for (auto& Input : InputsPLX)
	{
		if (auto Lvmvi =
				std::dynamic_pointer_cast<openplx::Physics::Signals::LinearVelocity1DInput>(Input))
		{
			Inputs.Add(FPLX_Input(Convert(Input->getName()), EPLX_InputType::LinearVelocity1DInput));
		}
		else
		{
			UE_LOG(LogAGX, Warning, TEXT("Unsupported PLX Input: %s"), *Convert(Input->getName()));
			Inputs.Add(
				FPLX_Input(Convert(Input->getName()), EPLX_InputType::Invalid));
		}
	}
	return Inputs;
}

TArray<FPLX_Output> FPLXUtilities::GetOutputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Output> Outputs;
	if (System == nullptr)
		return Outputs;

	auto OutputsPLX = GetNestedObjects<openplx::Physics::Signals::Output>(*System);
	Outputs.Reserve(OutputsPLX.size());
	for (auto& Output : OutputsPLX)
	{
		if (auto Hao = std::dynamic_pointer_cast<openplx::Physics::Signals::AngleOutput>(Output))
		{
			Outputs.Add(FPLX_Output(Convert(Output->getName()), EPLX_OutputType::AngleOutput));
		}
		else
		{
			UE_LOG(LogAGX, Warning, TEXT("Unsupported PLX Output: %s"), *Convert(Output->getName()));
			Outputs.Add(FPLX_Output(Convert(Output->getName()), EPLX_OutputType::Invalid));
		}
	}
	return Outputs;
}

std::unordered_set<openplx::Core::ObjectPtr> FPLXUtilities::GetNestedObjectFields(
	openplx::Core::Object& Object)
{
	std::unordered_set<openplx::Core::ObjectPtr> ObjectFields;
	GetNestedObjectFields(Object, ObjectFields);
	return ObjectFields;
}

void FPLXUtilities::GetNestedObjectFields(
	openplx::Core::Object& Object, std::unordered_set<openplx::Core::ObjectPtr>& Output)
{
	std::vector<openplx::Core::ObjectPtr> Fields = GetObjectFields(Object);
	for (auto& Field : Fields)
	{
		if (Field == nullptr)
			continue;
		if (Output.find(Field) != Output.end())
			continue;

		Output.insert(Field);
		GetNestedObjectFields(*Field, Output);
	}
}

std::vector<openplx::Core::ObjectPtr> FPLXUtilities::GetObjectFields(openplx::Core::Object& Object)
{
	std::vector<openplx::Core::ObjectPtr> Result;
	Result.reserve(100); // TODO: DONT MERGE! call freeContainerMemory instead once it is available.

	if (auto System = dynamic_cast<openplx::Physics3D::System*>(&Object))
	{
		// See openplx::Physics3D::System::extractObjectFieldsTo.
		Result.push_back(System->local_transform());
		Result.push_back(System->reference_body());
	}

	
	Object.extractObjectFieldsTo(Result);
	return Result;
}
