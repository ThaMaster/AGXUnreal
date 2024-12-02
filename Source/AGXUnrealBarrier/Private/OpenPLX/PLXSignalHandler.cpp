// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"

// OpenPLX includes.
#include "OpenPLX/agx-openplx/SignalListenerUtils.h"
#include "OpenPLX/Physics/Signals/RealInputSignal.h"

void FPLXSignalHandler::Init(
	const FString& PLXFile, const FString& UniqueModelInstancePrefix,
	FSimulationBarrier& Simulation, FPLXModelRegistry& InModelInfo,
	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());
	check(InModelInfo.HasNative());

	agxSDK::AssemblyRef Assembly = new agxSDK::Assembly();
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		Assembly->add(Constraint->GetNative()->Native);
	}

	// OpenPLX OutputSignalListener requires the assembly to contain a PowerLine with a
	// cetain name. Remove once this has been cleaned up in OpenPLX, it's a bit hacky.
	agxPowerLine::PowerLineRef RequiredDummyPowerLine = new agxPowerLine::PowerLine();
	RequiredDummyPowerLine->setName(agx::Name("OpenPlxPowerLine"));
	Assembly->add(RequiredDummyPowerLine);

	FAssemblyRef AssemblyRef(Assembly);
	ModelInfo = &InModelInfo;
	ModelHandle = ModelInfo->Register(PLXFile, UniqueModelInstancePrefix, AssemblyRef, Simulation);
	if (ModelHandle == FPLXModelRegistry::InvalidHandle)
	{
		// Todo: log error
		return;
	}

	bIsInitialized = true;
}

bool FPLXSignalHandler::IsInitialized() const
{
	return bIsInitialized;
}

bool FPLXSignalHandler::Send(const FPLX_LinearVelocity1DInput& Input, double Value)
{
	check(IsInitialized());
	if (ModelInfo == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	FPLXModelDatum* ModelDatum = ModelInfo->GetModelDatum(ModelHandle);
	if (ModelDatum == nullptr)
		return false;

	auto PLXInput = ModelDatum->Inputs.find(Convert(Input.Name));
	if (PLXInput == ModelDatum->Inputs.end())
		return false;

	auto Signal = openplx::Physics::Signals::RealInputSignal::create(
		ConvertDistanceToAGX(Value), PLXInput->second);

	// Todo: prepend unique instance name prefix to signal once supported.
	//agxopenplx::Signals::sendInputSignal(Signal); // // TODO!!!!
	return true;
}

bool FPLXSignalHandler::Receive(const FPLX_AngleOutput& Output, double& OutValue)
{
	check(IsInitialized());
	if (ModelInfo == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	// Todo: make this more efficient than looking through all outputs every time.
	// We could build a map from name to signal in the ModelRegistry, each time step, for example.
	// Do this once the signal refactoring in OpenPLX has been done.
	/*auto ValueOutputSignal = // // TODO!!!!
		agxopenplx::getSignalBySourceName<openplx::Physics::Signals::ValueOutputSignal>( // TODO!!!!
			agxopenplx::Signals::getOutputSignals(), Convert(Output.Name)); // TODO!!!!
	if (ValueOutputSignal == nullptr)
		return false;

	auto Value =
		std::dynamic_pointer_cast<openplx::Physics::Signals::RealValue>(ValueOutputSignal->value()); // TODO!!!!
	if (Value == nullptr)
		return false; // TODO!!!!

	OutValue = ConvertAngleToUnreal<double>(Value->value());*/ // TODO!!!!
	return true;
}
