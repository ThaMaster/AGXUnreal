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

bool FPLXSignalHandler::Send(const FPLX_LinearVelocityMotorVelocityInput& Input, double Value)
{
	check(IsInitialized());
	if (ModelInfo == nullptr)
		return false;

	FPLXModelDatum* ModelDatum = ModelInfo->GetModelDatum(ModelHandle);
	if (ModelDatum == nullptr)
		return false;

	auto PLXInput = ModelDatum->Inputs.find(Convert(Input.Name));
	if (PLXInput == ModelDatum->Inputs.end())
		return false;

	auto Signal =
		Brick::Physics::Signals::RealInputSignal::create(ConvertDistanceToAGX(Value), PLXInput->second);

	// Todo: prepend unique instance name prefix to signal once supported.
	BrickAgx::Signals::sendInputSignal(Signal);
	return true;

	return false;
}
