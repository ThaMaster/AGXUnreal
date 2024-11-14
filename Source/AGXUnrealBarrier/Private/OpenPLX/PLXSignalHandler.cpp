// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"

// OpenPLX includes.
#include "Brick/brickagx/Signals.h"
#include "Brick/Physics/Signals/RealInputSignal.h"
#include "Brick/Physics3D/Signals/LinearVelocityMotorVelocityInput.h"


void FPLXSignalHandler::Init(
	const FString& PLXFile, FSimulationBarrier& Simulation,
	FPLXModelRegistry& InModelInfo,	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());
	check(InModelInfo.HasNative());

	agxSDK::AssemblyRef Assembly = new agxSDK::Assembly();
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		Assembly->add(Constraint->GetNative()->Native);
	}

	FAssemblyRef AssemblyRef(Assembly);
	ModelInfo = &InModelInfo;
	ModelHandle = ModelInfo->Register(PLXFile, AssemblyRef, Simulation);
	if (ModelHandle == FPLXModelRegistry::InvalidHandle)
	{
		// Todo: log error
		return;
	}
}

bool FPLXSignalHandler::IsInitialized() const
{
	return true;// Todo, uncomment below once implemented.
	//return NativeOutputSignalHandlerRef != nullptr;
}

void FPLXSignalHandler::ReleaseNatives()
{
	NativeOutputSignalHandlerRef = nullptr;
}

namespace PLXSignalHandler_helpers
{
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
			OutSignalInputs.push_back(SignalInput);
		}
	}
}

bool FPLXSignalHandler::Send(const FPLX_LinearVelocityMotorVelocityInput& Input, double Value)
{
	AGX_CHECK(IsInitialized());
	if (ModelInfo == nullptr)
		return false;

	const FPLXModelDatum* ModelDatum = ModelInfo->GetModelDatum(ModelHandle);
	if (ModelDatum == nullptr)
		return false;

	std::vector<std::shared_ptr<Brick::Physics::Signals::Input>> SignalInputs;
	auto System = std::dynamic_pointer_cast<Brick::Physics3D::System>(ModelDatum->PLXModel);
	if (System == nullptr)
		return false;

	PLXSignalHandler_helpers::findAllSignalInputs(System.get(), SignalInputs);

	// Todo - map these in advance.
	for (std::shared_ptr<Brick::Physics::Signals::Input>& SignalInput : SignalInputs)
	{
		auto LinearVelInput =
			std::dynamic_pointer_cast<Brick::Physics3D::Signals::LinearVelocityMotorVelocityInput>(
				SignalInput);
		if (LinearVelInput == nullptr)
			continue;

		auto Signal = Brick::Physics::Signals::RealInputSignal::create(ConvertDistanceToAGX(Value), LinearVelInput);
		BrickAgx::Signals::sendInputSignal(Signal);
		return true;
	}

	return false;
}
