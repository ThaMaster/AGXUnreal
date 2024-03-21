// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RayPatternGenerator.h>
#include "EndAGXIncludes.h"

class FCustomPatternFetcherBase;

class FCustomPatternGenerator : public agxSensor::RayPatternGenerator
{
public:
	FCustomPatternGenerator() = default;
	FCustomPatternGenerator(FCustomPatternFetcherBase* InFetcher);

	void SetCustomPatternFetcher(FCustomPatternFetcherBase* InFetcher);
	FCustomPatternFetcherBase* GetCustomPatternFetcher() const;

	virtual agxSensor::RayPatternInterval getNextInterval(agx::Real dt) override;

private:
	FCustomPatternFetcherBase* PatternFetcher {nullptr};
};
