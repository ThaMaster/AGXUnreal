// Copyright 2024, Algoryx Simulation AB.

#pragma once


// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FInputSignalHandlerRef;
struct FPLX_LinearVelocityMotorVelocityInput;

class AGXUNREALBARRIER_API FPLXSignalHandler
{
public:
	void Init(const FString& PLXFile, FSimulationBarrier& Simulation, TArray<FConstraintBarrier*>& Constraints);

	bool IsInitialized() const;

	void ReleaseNatives();

	// todo: match the base class RealInput on the PLX side for less overloads.
	bool Send(FPLX_LinearVelocityMotorVelocityInput Input, double Value);

private: 
	std::shared_ptr<FAssemblyRef> NativeAssemblyRef;
	std::shared_ptr<FInputSignalHandlerRef> NativeInputSignalHandlerRef;
};
