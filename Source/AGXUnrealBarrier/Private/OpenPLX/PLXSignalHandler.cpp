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
#include "Brick/brickagx/InputSignalListener.h"
#include "Brick/brickagx/Signals.h"
#include "Brick/Physics/Signals/RealInputSignal.h"
#include "Brick/Physics3D/Signals/LinearVelocityMotorVelocityInput.h"

static Brick::Core::ObjectPtr PLXModel = nullptr; // Todo: store somewhere to be re-used!

void FPLXSignalHandler::Init(
	const FString& PLXFile, FSimulationBarrier& Simulation,
	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());

	// Todo: this PLXModel (tree) should be cached somewhere and re-used for other instances of the
	// same PLX model in the same world, so that LoadModel only has to be done once for any givel
	// PLX model.
	std::shared_ptr<BrickAgx::AgxCache> AGXCache;
	PLXModel = FPLXUtilities::LoadModel(PLXFile, AGXCache);
	if (PLXModel == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not read OpenPLX file '%s'. The Log category LogAGXDynamics may include "
				 "more details."),
			*PLXFile);
		return;
	}

	// Here, we re-use the InputSignalListener in OpenPLX to propagate signals.
	// It expects an assembly with the relevant contraints (and powerlines).
	// If no powerline exists in the OpenPLX model, one must be added anyway because the
	// InputSignalListener assumes one.
	NativeAssemblyRef = std::make_shared<FAssemblyRef>(new agxSDK::Assembly());
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		NativeAssemblyRef->Native->add(Constraint->GetNative()->Native);
	}

	agxPowerLine::PowerLineRef RequiredDummyPowerLine = new agxPowerLine::PowerLine();
	RequiredDummyPowerLine->setName(agx::Name("BrickPowerLine"));
	NativeAssemblyRef->Native->add(RequiredDummyPowerLine);
	NativeInputSignalHandlerRef =
		std::make_shared<FInputSignalHandlerRef>(NativeAssemblyRef->Native);
	Simulation.GetNative()->Native->add(NativeInputSignalHandlerRef->Native.get());

	// Todo: build up signals map for fast lookup later.
}

bool FPLXSignalHandler::IsInitialized() const
{
	return NativeAssemblyRef != nullptr && NativeInputSignalHandlerRef != nullptr;
}

void FPLXSignalHandler::ReleaseNatives()
{
	NativeInputSignalHandlerRef = nullptr;
	PLXModel = nullptr;
	NativeAssemblyRef = nullptr;
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

bool FPLXSignalHandler::Send(FPLX_LinearVelocityMotorVelocityInput Input, double Value)
{
	AGX_CHECK(IsInitialized());

	std::vector<std::shared_ptr<Brick::Physics::Signals::Input>> SignalInputs;
	auto System = std::dynamic_pointer_cast<Brick::Physics3D::System>(PLXModel);
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
