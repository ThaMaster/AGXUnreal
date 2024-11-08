#pragma once

// Never merge this to master. Only for development experiments/tests.
// TODO @todo (just to make this file be found)

// Brick includes.
#include "Brick/brick/BrickContext.h"
#include "Brick/brick/BrickContextInternal.h"
#include "Brick/Brick/BrickCoreApi.h"
#include "Brick/brickagx/AgxCache.h"
#include "Brick/brickagx/BrickAgxApi.h"
#include "Brick/brickagx/BrickToAgxMapper.h"
#include "Brick/brickagx/InputSignalListener.h"
#include "Brick/brickagx/Signals.h"
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


std::shared_ptr<Brick::Core::Api::BrickContext> CreateBrickContext(
	std::shared_ptr<BrickAgx::AgxCache> AGXCache)
{
	const FString BrickBundlesPath = FPaths::Combine(
		FAGX_Environment::GetPluginSourcePath(), "Thirdparty", "agx", "brickbundles");
	auto BrickCtx = std::make_shared<Brick::Core::Api::BrickContext>(
		std::vector<std::string>({Convert(BrickBundlesPath)}));

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

	BrickAgx::register_plugins(*BrickCtx, AGXCache);
	return BrickCtx;
}

Brick::Core::ObjectPtr LoadModelFromFile(
	const std::string& BrickFile, std::shared_ptr<BrickAgx::AgxCache> AGXCache)
{
	auto Context = CreateBrickContext(AGXCache);
	if (Context == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Error Creating Brick Context"));
		return nullptr;
	}

	auto LoadedModel = Brick::Core::Api::loadModelFromFile(BrickFile, {}, *Context);

	if (Context->hasErrors())
	{
		LoadedModel = nullptr;
		for (auto Error : Context->getErrors())
			UE_LOG(LogAGX, Error, TEXT("Error in Brick Context : %d"), Error->getErrorCode());

		return nullptr;
	}

	return LoadedModel;
}

static Brick::Core::ObjectPtr BrickModel = nullptr;
static std::shared_ptr<BrickAgx::AgxCache> AGXCache = nullptr;
static std::vector<std::shared_ptr<Brick::Physics::Signals::Input>> SignalInputs;
static agxSDK::AssemblyRef Assembly;
static std::shared_ptr<BrickAgx::InputSignalListener> InputSignalListener;

void findAllSignalInputs(
	Brick::Physics3D::System* System,
	std::vector<std::shared_ptr<Brick::Physics::Signals::Input>>& OutSignalInputs)
{
	for (auto& Subsystem : System->getValues<Brick::Physics3D::System>())
	{
		findAllSignalInputs(System, OutSignalInputs);
	}

	for (auto& SignalInput : System->getValues<Brick::Physics::Signals::Input>())
	{
		SignalInputs.push_back(SignalInput);
	}
}

void BrickExperimentInit(agxSDK::Simulation* Simulation)
{
	const FString Filename = "C:/Users/Admin/Desktop/inverted_pendulum.brick";
	BrickModel = LoadModelFromFile(Convert(Filename), AGXCache);
	if (BrickModel == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not read Brick file '%s'. The Log category LogAGXDynamics may include more "
				 "details."),
			*Filename);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("BrickExperimentInit %d"), Simulation != nullptr);

	auto System = std::dynamic_pointer_cast<Brick::Physics3D::System>(BrickModel);
	findAllSignalInputs(System.get(), SignalInputs);

	for (auto& Constraint : Simulation->getConstraints())
	{
		for (size_t i = 0; i < Constraint->getNumSecondaryConstraints(); i++)
		{
			agx::ElementaryConstraint* ElemC = Constraint->getSecondaryConstraint(i);
			agx::TargetSpeedController* TargetSC = dynamic_cast<agx::TargetSpeedController*>(ElemC);
			if (TargetSC == nullptr)
				continue;

			TargetSC->setName(agx::Name("PendulumScene.cart_motor"));
			UE_LOG(
				LogTemp, Warning, TEXT("Elem Constraint %s is targetspeed: %d"),
				*Convert(ElemC->getName()), TargetSC != nullptr);

			Assembly = new agxSDK::Assembly();
			Assembly->add(Constraint);
		}
	}

	if (Assembly->getConstraints().size() == 0)
		return;

	// Hacky hacky hack-hack start
	agxPowerLine::PowerLineRef BrickPowerLineHack = new agxPowerLine::PowerLine();
	BrickPowerLineHack->setName(agx::Name("BrickPowerLine"));
	Assembly->add(BrickPowerLineHack);
	// Hacky hacky hack-hack end

	InputSignalListener = std::make_shared<BrickAgx::InputSignalListener>(Assembly);
	Simulation->add(InputSignalListener.get());
}

void BrickExperimentCheck(agxSDK::Simulation* Simulation)
{
	UE_LOG(LogTemp, Warning, TEXT("BrickExperimentCheck %d"), Simulation != nullptr);
	if (SignalInputs.size() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Num signal inputs found in check: %d"), SignalInputs.size());
		return;
	}

	std::shared_ptr<Brick::Physics::Signals::RealInputSignal> MyCoolInputSignal;

	for (std::shared_ptr<Brick::Physics::Signals::Input>& SignalInput : SignalInputs)
	{
		auto LinearVelInput =
			std::dynamic_pointer_cast<Brick::Physics3D::Signals::LinearVelocityMotorVelocityInput>(
				SignalInput);
		if (LinearVelInput == nullptr)
			continue;

		UE_LOG(
			LogTemp, Warning, TEXT("Found LinearVelocityMotorVelocityInput! %s"),
			*Convert(LinearVelInput->getName()));
		UE_LOG(
			LogTemp, Warning, TEXT("Motor name: %s"), *Convert(LinearVelInput->motor()->getName()));

		MyCoolInputSignal = Brick::Physics::Signals::RealInputSignal::create(-1.0, LinearVelInput);
		BrickAgx::Signals::sendInputSignal(MyCoolInputSignal);
	}
}
