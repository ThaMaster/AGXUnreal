// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FSimulationBarrier;

struct FPLX_LinearVelocityMotorVelocityInput;
struct FOutputSignalHandlerRef;

class AGXUNREALBARRIER_API FPLXSignalHandler
{
public:
	void Init(
		const FString& PLXFile, FSimulationBarrier& Simulation, FPLXModelRegistry& InModelInfo,
		TArray<FConstraintBarrier*>& Constraints);

	bool IsInitialized() const;

	void ReleaseNatives();

	// todo: match the base class RealInput on the PLX side for less overloads.
	bool Send(const FPLX_LinearVelocityMotorVelocityInput& Input, double Value);

private:
	std::shared_ptr<FOutputSignalHandlerRef> NativeOutputSignalHandlerRef;
	FPLXModelRegistry* ModelInfo {nullptr};

	FPLXModelRegistry::Handle ModelHandle {FPLXModelRegistry::InvalidHandle};
};
