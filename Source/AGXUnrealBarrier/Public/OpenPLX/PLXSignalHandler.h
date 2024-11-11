// Copyright 2024, Algoryx Simulation AB.

#pragma once

class FConstraintBarrier;
class FSimulationBarrier;

class AGXUNREALBARRIER_API FPLXSignalHandler
{
public:
	void Init(FSimulationBarrier& Simulation, TArray<FConstraintBarrier*>& Constraints);
};
