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
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/SignalListenerUtils.h"
#include "Physics/Signals/RealInputSignal.h"
#include "EndAGXIncludes.h"

FPLXSignalHandler::FPLXSignalHandler()
{
}

void FPLXSignalHandler::Init(
	const FString& PLXFile, FSimulationBarrier& Simulation, FPLXModelRegistry& InModelRegistry,
	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());
	check(InModelRegistry.HasNative());

	AssemblyRef = std::make_shared<FAssemblyRef>(new agxSDK::Assembly());
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		AssemblyRef->Native->add(Constraint->GetNative()->Native);
	}

	// OpenPLX OutputSignalListener requires the assembly to contain a PowerLine with a
	// certain name. Remove once this has been cleaned up in OpenPLX, it's a bit hacky.
	agxPowerLine::PowerLineRef RequiredDummyPowerLine = new agxPowerLine::PowerLine();
	RequiredDummyPowerLine->setName(agx::Name("OpenPlxPowerLine"));
	AssemblyRef->Native->add(RequiredDummyPowerLine);

	ModelRegistry = &InModelRegistry;
	ModelHandle = ModelRegistry->Register(PLXFile);
	if (ModelHandle == FPLXModelRegistry::InvalidHandle)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Could not load OpenPLX model '%s'. The Console Log may contain more information."),
			*PLXFile);
		return;
	}

	FPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Unable to get registered OpenPLX model '%s'. The OpenPLX model "
				 "may not behave as intended."),
			*PLXFile);
		return;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(ModelData->PLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get a openplx::Physics3D::System from the registered OpenPLX model "
				 "'%s'. The OpenPLX model may not behave as intended."),
			*PLXFile);
		return;
	}

	if (FPLXUtilities::HasInputs(System.get()))
	{
		InputQueueRef =
			std::make_shared<FInputSignalQueueRef>(agxopenplx::InputSignalQueue::create());
		InputSignalHandlerRef =
			std::make_shared<FInputSignalHandlerRef>(AssemblyRef->Native, InputQueueRef->Native);
		Simulation.GetNative()->Native->add(InputSignalHandlerRef->Native);
	}

	if (FPLXUtilities::HasOutputs(System.get()))
	{
		OutputQueueRef =
			std::make_shared<FOutputSignalQueueRef>(agxopenplx::OutputSignalQueue::create());
		OutputSignalHandlerRef = std::make_shared<FOutputSignalHandlerRef>(
			AssemblyRef->Native, ModelData->PLXModel, OutputQueueRef->Native);
		Simulation.GetNative()->Native->add(OutputSignalHandlerRef->Native);
	}

	bIsInitialized = true;
}

bool FPLXSignalHandler::IsInitialized() const
{
	return bIsInitialized;
}

namespace PLXSignalHandler_helpers
{
	TOptional<double> ConvertScalar(const FPLX_Input& Input, double Value)
	{
		switch (Input.Type)
		{
			case EPLX_InputType::AngleInput:
			case EPLX_InputType::AngularVelocity1DInput:
				return ConvertAngleToAGX(Value);
			case EPLX_InputType::DurationInput:
			case EPLX_InputType::AutomaticClutchEngagementDurationInput:
			case EPLX_InputType::AutomaticClutchDisengagementDurationInput:
			case EPLX_InputType::FractionInput:
			case EPLX_InputType::Force1DInput:
			case EPLX_InputType::Torque1DInput:
				return Value;
			case EPLX_InputType::Position1DInput:
			case EPLX_InputType::LinearVelocity1DInput:
				return ConvertDistanceToAGX(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert scalar value for Input '%s', but the type is either "
				 "not of scalar type or is unsupported."),
			*Input.Name);
		return {};
	}
}

bool FPLXSignalHandler::Send(const FPLX_Input& Input, double Value)
{
	check(IsInitialized());
	if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	if (InputQueueRef == nullptr || InputQueueRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to send OpenPLX Input signal for Input '%s', but the OpenPLX "
				 "model does not have any registered Inputs."),
			*Input.Name);
		return false;
	}

	FPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
		return false;

	auto PLXInput = ModelData->Inputs.find(Convert(Input.Name));
	if (PLXInput == ModelData->Inputs.end())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to send OpenPLX signal, value %f, but the corresponding OpenPLX Input '%s' "
				 "was not found in the model. The signal will not be sent."),
			Value, *Input.Name);
		return false;
	}

	auto ConvertedValue = PLXSignalHandler_helpers::ConvertScalar(Input, Value);
	if (!ConvertedValue.IsSet())
		return false;

	auto Signal =
		openplx::Physics::Signals::RealInputSignal::create(*ConvertedValue, PLXInput->second);
	InputQueueRef->Native->send(Signal);
	return true;
}

namespace PLXSignalHandler_helpers
{
	TOptional<double> GetScalarValueFrom(
		const FPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto TypeMismatchResult = [&]() -> TOptional<double>
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unexpected error: Tried to cast OpenPLX Output '%s' to it's corresponding "
					 "OpenPLX type but got nullptr. Possible type miss match. The signal will not "
					 "be received."),
				*Output.Name);
			return {};
		};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::RealValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult();

		switch (Output.Type)
		{
			
			case EPLX_OutputType::DurationOutput:
			case EPLX_OutputType::AutomaticClutchEngagementDurationOutput:
			case EPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
			case EPLX_OutputType::FractionOutput:
			case EPLX_OutputType::Force1DOutput:
			case EPLX_OutputType::Torque1DOutput:
			case EPLX_OutputType::TorqueConverterPumpTorqueOutput:
			case EPLX_OutputType::TorqueConverterTurbineTorqueOutput:
				return Value->value();
			case EPLX_OutputType::AngleOutput:
			case EPLX_OutputType::AngularVelocity1DOutput:
				return ConvertAngleToUnreal<double>(Value->value());
			case EPLX_OutputType::LinearVelocity1DOutput:
			case EPLX_OutputType::Position1DOutput:
			case EPLX_OutputType::RelativeVelocity1DOutput:
				return ConvertDistanceToUnreal<double>(Value->value());
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read scalar type from signal for Output '%s', but the type is either "
				 "not of scalar type or is unsupported."),
			*Output.Name);
		return {};
	}
}

bool FPLXSignalHandler::Receive(const FPLX_Output& Output, double& OutValue)
{
	check(IsInitialized());
	if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	if (OutputQueueRef == nullptr || OutputQueueRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to receive OpenPLX Angle Output signal for output '%s', but the OpenPLX "
				 "model does not have any registered outputs."),
			*Output.Name);
		return false;
	}

	auto Signal = agxopenplx::getSignalBySourceName<openplx::Physics::Signals::ValueOutputSignal>(
		OutputQueueRef->Native->getSignals(), Convert(Output.Name));
	if (Signal == nullptr)
		return false;

	auto Value = PLXSignalHandler_helpers::GetScalarValueFrom(Output, Signal.get());
	if (!Value.IsSet())
		return false;

	OutValue = *Value;
	return true;
}
