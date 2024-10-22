// Copyright 2024, Algoryx Simulation AB.

#include "Brick/Brick.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"

// Brick includes.
#include "Brick/brick/BrickContext.h"
#include "Brick/brick/BrickContextInternal.h"
#include "Brick/Brick/BrickCoreApi.h"
#include "Brick/brickagx/AgxCache.h"
#include "Brick/brickagx/BrickAgxApi.h"
#include "Brick/Math/Math_all.h"
#include "Brick/Physics/Physics_all.h"
#include "Brick/Physics1D/Physics1D_all.h"
#include "Brick/Physics3D/Physics3D_all.h"
#include "Brick/DriveTrain/DriveTrain_all.h"
#include "Brick/Robotics/Robotics_all.h"
#include "Brick/Simulation/Simulation_all.h"
#include "Brick/Vehicles/Vehicles_all.h"
#include "Brick/Terrain/Terrain_all.h"
#include "Brick/Visuals/Visuals_all.h"
#include "Brick/Urdf/Urdf_all.h"


// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "EndAGXIncludes.h"

#include <memory>
#include <vector>
#include <string>

namespace
{
	class BrickUnrealMapper
	{
	public:
		std::shared_ptr<BrickAgx::AgxCache> AGXCache;

		void MapModel(Brick::Core::ObjectPtr Model)
		{
			// Todo.
			UE_LOG(LogTemp, Warning, TEXT("MapModel called!"));
		}

	private:
	};

	std::shared_ptr<Brick::Core::Api::BrickContext> CreateBrickContext(BrickUnrealMapper& Mapper)
	{
		auto BrickCtx = std::make_shared<Brick::Core::Api::BrickContext>(std::vector<std::string>(
			{"C:/Users/Admin/git/agxunreal/AGXUnrealDev/Plugins/AGXUnreal/Source/ThirdParty/agx/brickbundles"}));

		auto InternalContext = Brick::Core::Api::BrickContextInternal::fromContext(*BrickCtx);
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

		BrickAgx::register_plugins(*BrickCtx, Mapper.AGXCache);
		return BrickCtx;
	}

	Brick::Core::ObjectPtr ParseBrickSource(
		const std::filesystem::path& BrickFile, BrickUnrealMapper& Mapper)
	{
		auto Context = CreateBrickContext(Mapper);
		if (Context == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Error Creating Brick Context"));
			return nullptr;
		}

		auto LoadedModel = Brick::Core::Api::loadModelFromFile(BrickFile, {}, *Context);

		if (Context->hasErrors())
		{
			LoadedModel = nullptr;
			for (auto Error : Context->getErrors())
				UE_LOG(LogTemp, Error, TEXT("Error in Brick Context : %d"), Error->getErrorCode());
		}

		return LoadedModel;
	}

	void ImportBrickFile(const std::filesystem::path& BrickFile)
	{
		BrickUnrealMapper Mapper;
		auto LoadedModel = ParseBrickSource(BrickFile, Mapper);
		if (LoadedModel == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to load model %s"), *Convert(BrickFile.string()));
			return;
		}

		Mapper.MapModel(LoadedModel);
	}
}

void FBrick::Test()
{
	::ImportBrickFile("C:/Users/Admin/Desktop/double_pendulum.brick");
}
